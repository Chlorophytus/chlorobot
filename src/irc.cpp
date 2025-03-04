#include "../include/irc.hpp"
#include "../include/scripting.hpp"
using namespace chlorobot;

static bool running = true;

void irc::connect(std::string &&host, std::string &&port,
                  irc_data::user &&_data) {
  const auto data = _data;

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

  chlorobot::scripting::start(std::filesystem::path{"."});

  while (running) {
    const auto recv = tls_socket::recv();

    if (recv) {
      const auto packets = irc_data::packet::parse(*recv);
      for (auto packet : packets) {
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
        chlorobot::scripting::maybe_handle_packet(packet);
      }
    }
  }
  std::cerr << "Runloop halted, disconnecting socket"
            << std::endl;

  tls_socket::send(irc_data::packet{.command = "QUIT",
                                    .trailing_param = "Chlorobot"}
                       .serialize());
  chlorobot::scripting::stop();
  tls_socket::disconnect();
}
