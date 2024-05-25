#include "../include/irc.hpp"
using namespace chlorobot;

bool running = true;
std::unique_ptr<std::set<irc::authentication *>> listener_tags = nullptr;
std::unique_ptr<std::string> rpc_token = nullptr;

// duration to wait to get a RPC message
constexpr auto rpc_deadline = std::chrono::milliseconds(1);


irc::request::request(ChlorobotRPC::AsyncService *service,
                      grpc::ServerCompletionQueue *queue)
    : _service(service), _completion_queue(queue), _responder(&_context) {
  proceed();
}
void irc::request::proceed() {
  if (_state == irc::async_state::create) {
    _state = irc::async_state::process;
    _service->RequestSend(&_context, &_request, &_responder, _completion_queue,
                          _completion_queue, this);
  } else if (_state == irc::async_state::process) {
    new irc::request(_service, _completion_queue);

    if (_request.auth().token() == *rpc_token) {
      switch (_request.data_case()) {
      case ChlorobotRequest::DataCase::kCommandType: {
        switch (_request.command_type()) {
        case ChlorobotCommandEnum::SEND_VERSION: {
          auto version_set = _acknowledgement.mutable_version();
          version_set->set_major(chlorobot_VMAJOR);
          version_set->set_minor(chlorobot_VMINOR);
          version_set->set_patch(chlorobot_VPATCH);
          version_set->set_pretty(chlorobot_VSTRING_FULL);
          break;
        }

        default: {
          break;
        }
        }
        break;
      }
      case ChlorobotRequest::DataCase::kPacket: {
        const auto source_packet = _request.packet();
        auto dest_packet = irc_data::packet{};
        if (source_packet.has_prefix()) {
          dest_packet.prefix = source_packet.prefix();
        }
        switch (source_packet.command_case()) {
        case ChlorobotPacket::CommandCase::kNonNumeric: {
          dest_packet.command = source_packet.non_numeric();
          break;
        }
        case ChlorobotPacket::CommandCase::kNumeric: {
          dest_packet.command = source_packet.numeric();
          break;
        }
        default: {
          throw std::runtime_error{"Unimplemented gRPC request command case"};
          break;
        }
        }
        auto param_s = source_packet.parameters_size();
        for (auto param_i = 0; param_i < param_s; param_i++) {
          dest_packet.params.push_back(source_packet.parameters(param_i));
        }
        if (source_packet.has_trailing_parameter()) {
          dest_packet.trailing_param = source_packet.trailing_parameter();
        }
        tls_socket::send(dest_packet.serialize());
        break;
      }
      default: {
        throw std::runtime_error{"Unimplemented gRPC request data case"};
        break;
      }
      }

      _state = irc::async_state::finish;
      _responder.Finish(_acknowledgement, grpc::Status::OK, this);
    } else {
      _state = irc::async_state::finish;
      _responder.Finish(_acknowledgement, grpc::Status::CANCELLED, this);
    }
  } else {
    delete this;
  }
}

irc::authentication::authentication(ChlorobotRPC::AsyncService *service,
                                    grpc::ServerCompletionQueue *queue)
    : _service(service), _completion_queue(queue), _responder(&_context) {
  proceed();
}
void irc::authentication::proceed() {
  if (_state == irc::async_state::create) {
    _state = irc::async_state::process;
    _context.AsyncNotifyWhenDone(this);
    _service->RequestListen(&_context, &_authentication, &_responder,
                            _completion_queue, _completion_queue, this);
  } else if (_state == irc::async_state::process) {
    std::cerr << "Started listening: " << this << std::endl;
    new irc::authentication(_service, _completion_queue);
    listener_tags->insert(this);
    _state = irc::async_state::finish;
  } else {
    if (_context.IsCancelled()) {
      std::cerr << "Cancelled listening: " << this << std::endl;
      listener_tags->erase(this);
      delete this;
    }
  }
}
void irc::authentication::broadcast(const ChlorobotPacket packet) {
  _responder.Write(packet, this);
}

void irc::connect(std::string &&host, std::string &&port,
                  irc_data::user &&_data, std::string &&_rpc_token) {
  const auto data = _data;
  listener_tags = std::make_unique<std::set<irc::authentication *>>();
  rpc_token = std::make_unique<std::string>(_rpc_token);

  std::cerr << "Starting gRPC server" << std::endl;

  ChlorobotRPC::AsyncService rpc_service;
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  grpc::ServerBuilder rpc_builder;
  rpc_builder.AddListeningPort("0.0.0.0:50051",
                               grpc::InsecureServerCredentials());
  rpc_builder.RegisterService(&rpc_service);
  auto send_queue = rpc_builder.AddCompletionQueue();
  auto recv_queue = rpc_builder.AddCompletionQueue();
  auto server = rpc_builder.BuildAndStart();

  std::cerr << "Starting IRC connection" << std::endl;

  tls_socket::connect(host, port);

  auto attempt = 0;

  while (!tls_socket::recv()) {
    if (attempt < 10) {
      std::cerr << "Waiting on socket" << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      attempt++;
    } else {
      throw std::runtime_error{"Socket wait timed out, terminating"};
    }
  }
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

  std::cerr << "Starting Run Loop" << std::endl;

  new irc::request(&rpc_service, send_queue.get());
  new irc::authentication(&rpc_service, recv_queue.get());

  while (running) {
    // Check if we got a send message
    void *send_tag;
    bool send_ok;
    const auto send_rpc_status = send_queue->AsyncNext(
        &send_tag, &send_ok, std::chrono::system_clock::now() + rpc_deadline);
    if (send_rpc_status == grpc::CompletionQueue::GOT_EVENT) {
      if (send_ok) {
        static_cast<irc::request *>(send_tag)->proceed();
      }
    }

    void *recv_tag;
    bool recv_ok;
    const auto recv_rpc_status = recv_queue->AsyncNext(
        &recv_tag, &recv_ok, std::chrono::system_clock::now() + rpc_deadline);
    if (recv_rpc_status == grpc::CompletionQueue::GOT_EVENT) {
      if (recv_ok) {
        static_cast<irc::authentication *>(recv_tag)->proceed();
      }
    }

    const auto recv = tls_socket::recv();

    if (recv) {
      const auto packets = irc_data::packet::parse(*recv);
      for (auto packet : packets) {
        ChlorobotPacket rpc_packet{};
        if (packet.prefix) {
          rpc_packet.set_prefix(packet.prefix.value());
        }
        switch (packet.command.index()) {
        case 0: {
          rpc_packet.set_numeric(std::get<U32>(packet.command));
          break;
        }
        case 1: {
          rpc_packet.set_non_numeric(std::get<std::string>(packet.command));
          break;
        }
        default: {
          throw std::runtime_error{"undefined packet command variant"};
        }
        }
        const auto parameters_size = packet.params.size();
        for (auto parameter_index = 0; parameter_index < parameters_size;
             parameter_index++) {
          rpc_packet.add_parameters(packet.params[parameter_index]);
        }
        if (packet.trailing_param) {
          rpc_packet.set_trailing_parameter(packet.trailing_param.value());
        }

        for (auto tag : *listener_tags) {
          tag->broadcast(rpc_packet);
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
  send_queue->Shutdown();
  recv_queue->Shutdown();
}