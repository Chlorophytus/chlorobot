#include "../include/irc.hpp"
using namespace chlorobot;

static bool running = true;
static std::unique_ptr<std::vector<irc::listener>> listeners = nullptr;

// duration to wait to get a RPC message
constexpr static auto rpc_deadline = gpr_timespec{
    .tv_sec = 0,
    .tv_nsec = 1'000'000,
};

void irc::connect(std::string &&host, std::string &&port,
                  irc_data::user &&_data, std::string &&_rpc_token) {
  const auto rpc_token = _rpc_token;
  const auto data = _data;
  listeners = std::make_unique<std::vector<irc::listener>>();

  tls_socket::connect(host, port);

  std::cerr << "--- Connected ---" << std::endl;

  // Request SASL
  std::cerr << "Requesting SASL" << std::endl;
  tls_socket::send(
      irc_data::packet{.command = "CAP", .params = {"LS"}}.serialize());

  // Nickname
  std::cerr << "Sending nickname" << std::endl;
  tls_socket::send(
      irc_data::packet{.command = "NICK", .params = {data.nickname}}
          .serialize());

  // Username
  std::cerr << "Sending username" << std::endl;
  tls_socket::send(irc_data::packet{.command = "USER",
                                    .params = {data.ident, "0", "*"},
                                    .trailing_param = data.real_name}
                       .serialize());

  // CAP SASL Acknowledge waiter
  std::cerr << "Waiting for SASL listing from server" << std::endl;
  bool waiting = true;
  while (waiting) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = irc_data::packet::parse(*recv);
      for (auto &&packet : packets) {
        const auto command = packet.command;
        if (command.index() == 1) {
          if (std::get<std::string>(command) == "CAP") {
            std::string trailing_param = packet.trailing_param.value_or("");
            for (auto &&cap : std::views::split(trailing_param, ' ')) {
              if (std::string{cap.begin(), cap.end()} == "sasl") {
                tls_socket::send(irc_data::packet{.command = "CAP",
                                                  .params = {"REQ"},
                                                  .trailing_param = "sasl"}
                                     .serialize());
                waiting = false;
                break;
              }
            }
            if (waiting) {
              throw std::runtime_error{
                  "This IRC server does not support SASL!"};
            }
          }
        }
      }
    }
  }

  std::cerr << "Waiting for SASL acknowledge from server" << std::endl;
  waiting = true;
  while (waiting) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = irc_data::packet::parse(*recv);
      for (auto &&packet : packets) {
        const auto command = packet.command;
        if (command.index() == 1 && std::get<std::string>(command) == "CAP" &&
            packet.params.at(1) == "ACK" &&
            packet.trailing_param.value_or("") == "sasl") {
          tls_socket::send(
              irc_data::packet{.command = "AUTHENTICATE", .params = {"PLAIN"}}
                  .serialize());
          waiting = false;
          break;
        }
      }
    }
  }

  std::cerr << "Waiting for SASL authentication initiation from server"
            << std::endl;
  waiting = true;
  while (waiting) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = irc_data::packet::parse(*recv);
      for (auto &&packet : packets) {
        const auto command = packet.command;
        if (command.index() == 1) {
          if (std::get<std::string>(command) == "AUTHENTICATE" &&
              packet.params.at(0) == "+") {
            // this is a bit tricky
            waiting = false;
            U8 base64_in[400]{0};
            U8 base64_out[50]{0};
            // authcid
            U32 i = 0;
            for (char in : data.sasl_account) {
              base64_in[i] = in;
              i++;
            }
            base64_in[i++] = 0;
            // authzid
            for (char in : data.sasl_account) {
              base64_in[i] = in;
              i++;
            }
            base64_in[i++] = 0;
            // passwd
            for (char in : data.sasl_password) {
              base64_in[i] = in;
              i++;
            }

            // 400 / 8 = 50
            EVP_EncodeBlock(base64_out, base64_in, 50);

            tls_socket::send(irc_data::packet{
                .command = "AUTHENTICATE",
                .params = {std::string{
                    reinterpret_cast<char *>(base64_out),
                    50}}}.serialize());
            tls_socket::send(
                irc_data::packet{.command = "CAP", .params = {"END"}}
                    .serialize());
            break;
          }
        }
      }
    }
  }
  std::cerr << "Starting gRPC server" << std::endl;

  ChlorobotRPC::AsyncService rpc_service;
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder rpc_builder;
  rpc_builder.AddListeningPort("127.0.0.1:50051",
                               grpc::InsecureServerCredentials());
  rpc_builder.RegisterService(&rpc_service);
  auto completion_queue = rpc_builder.AddCompletionQueue();
  auto server = rpc_builder.BuildAndStart();
  grpc::ServerContext rpc_context;

  std::cerr << "Starting Run Loop" << std::endl;

  while (running) {
    // Check if we got a send message
    void *rpc_tag;
    bool rpc_ok;
    auto rpc_status =
        completion_queue->AsyncNext(&rpc_tag, &rpc_ok, rpc_deadline);
    const auto recv = tls_socket::recv();

    do {
      if (rpc_ok &&
          rpc_status == grpc::CompletionQueue::NextStatus::GOT_EVENT) {
        // We got something, check what it is
        if (static_cast<ChlorobotRequest *>(rpc_tag) != nullptr) {
          ChlorobotRequest request;
          grpc::ServerAsyncResponseWriter<ChlorobotAcknowledgement> responder(
              &rpc_context);
          rpc_service.RequestSend(&rpc_context, &request, &responder,
                                  completion_queue.get(),
                                  completion_queue.get(), rpc_tag);
          if (request.auth().token() == rpc_token) {
            ChlorobotAcknowledgement acknowledgement;
            switch (request.data_case()) {
            case ChlorobotRequest::DataCase::kCommandType: {
              acknowledgement.mutable_version()->set_major(chlorobot_VMAJOR);
              acknowledgement.mutable_version()->set_minor(chlorobot_VMINOR);
              acknowledgement.mutable_version()->set_patch(chlorobot_VPATCH);
              acknowledgement.mutable_version()->set_pretty(
                  chlorobot_VSTRING_FULL);
              break;
            }
            case ChlorobotRequest::DataCase::kPacket: {
              const auto send_this = request.packet();
              const auto send_params = send_this.parameters();
              const auto send_size = send_params.size();

              irc_data::packet serialize{};

              if (send_this.has_prefix()) {
                serialize.prefix = send_this.prefix();
              }
              switch (send_this.command_case()) {
              case ChlorobotPacket::CommandCase::kNumeric: {
                serialize.command = send_this.numeric();
                break;
              }
              case ChlorobotPacket::CommandCase::kNonNumeric: {
                serialize.command = send_this.non_numeric();
                break;
              }
              default: {
                throw std::runtime_error{
                    "unimplemented IRC command object type"};
                break;
              }
              }
              for (auto send_i = 0; send_i < send_size; send_i++) {
                serialize.params.emplace_back(send_params[send_i]);
              }
              if (send_this.has_trailing_parameter()) {
                serialize.trailing_param = send_this.trailing_parameter();
              }

              tls_socket::send(serialize.serialize());
              break;
            }
            default: {
              throw std::runtime_error{"unimplemented Chlorobot request"};
              break;
            }
            }
            responder.Finish(acknowledgement, grpc::Status::OK, rpc_tag);
          } else {
            responder.Finish(ChlorobotAcknowledgement(),
                             grpc::Status::CANCELLED, rpc_tag);
          }
        }
        if (static_cast<ChlorobotAuthentication *>(rpc_tag) != nullptr) {
          ChlorobotAuthentication request;
          listeners->emplace_back();
          listeners->back().tag = rpc_tag;

          rpc_service.RequestListen(
              &rpc_context, &request, listeners->back().writer,
              completion_queue.get(), completion_queue.get(),
              listeners->back().tag);
          if (request.token() != rpc_token) {
            listeners->back().writer->Finish(grpc::Status::CANCELLED,
                                             listeners->back().tag);
            listeners->pop_back();
          }
        }
      }
      rpc_status = completion_queue->AsyncNext(&rpc_tag, &rpc_ok, rpc_deadline);
    } while (rpc_status == grpc::CompletionQueue::NextStatus::GOT_EVENT);

    if (recv) {
      const auto packets = irc_data::packet::parse(*recv);

      for (auto &&packet : packets) {
        // gRPC code
        ChlorobotPacket rpc_packet;
        if (packet.prefix) {
          rpc_packet.set_prefix(packet.prefix.value());
        }
        if (packet.command.index() == 1) {
          rpc_packet.set_non_numeric(std::get<std::string>(packet.command));
        } else {
          rpc_packet.set_numeric(std::get<U32>(packet.command));
        }
        const auto parameters_size = packet.params.size();
        for (auto parameter_index = 0; parameter_index < parameters_size;
             parameter_index++) {
          rpc_packet.set_parameters(parameter_index,
                                    packet.params[parameter_index]);
        }
        if (packet.trailing_param) {
          rpc_packet.set_trailing_parameter(packet.trailing_param.value());
        }
        for (auto &&rpc_listener : *listeners) {
          rpc_listener.writer->Write(rpc_packet, rpc_listener.tag);
        }

        // core code
        if (packet.command.index() == 1) {
          const auto command_str = std::get<std::string>(packet.command);
          if (command_str == "ERROR") {
            std::cerr << "Disconnected: "
                      << packet.trailing_param.value_or("???") << std::endl;
            running = false;
          }
          if (command_str == "PING") {
            tls_socket::send(irc_data::packet{
                .command = "PONG", .trailing_param = packet.trailing_param}
                                 .serialize());
          }
        }
      }
    }
  }
  std::cerr << "Runloop halted, disconnecting socket" << std::endl;
  tls_socket::send(irc_data::packet{.command = "QUIT",
                                    .trailing_param = "Run loop has halted"}
                       .serialize());
  tls_socket::disconnect();

  server->Shutdown();
  completion_queue->Shutdown();
}