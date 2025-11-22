#include "../include/main.hpp"
#include "../include/irc.hpp"
#include "../include/irc_data.hpp"
#include "../include/irc_sasl.hpp"
#include "../include/scripting.hpp"

int main(int argc, char **argv) {
  // Fail safe
  try {
    std::cerr << "Chlorobot " << chlorobot_VSTRING_FULL << std::endl;

    // This should be stored in a .env file!
    const std::string nickname = std::getenv("CHLOROBOT_NICKNAME");
    const std::string ident = std::getenv("CHLOROBOT_IDENT");
    const std::string real_name = std::getenv("CHLOROBOT_REALNAME");
    const std::string sasl_username = std::getenv("CHLOROBOT_SASL_USERNAME");
    const std::string sasl_password = std::getenv("CHLOROBOT_SASL_PASSWORD");

    chlorobot::tls_socket::socket &sock =
        chlorobot::tls_socket::socket::get_instance();

    const std::string host{std::getenv("CHLOROBOT_NETWORK_HOST")};
    const std::string port{std::getenv("CHLOROBOT_NETWORK_PORT")};

    sock.connect(host, port);
    chlorobot::irc::send_user_info(nickname, ident, real_name);
    
    if (chlorobot::irc_sasl::has_sasl_capability()) {
      chlorobot::irc_sasl::try_auth(sasl_username, sasl_password);
    } else {
      throw std::runtime_error{"SASL is not present on this server"};
    }

    chlorobot::scripting::engine &lua =
        chlorobot::scripting::engine::get_instance();

    while (sock) {
      auto received = sock.recv();

      if (received) {
        const auto packets = chlorobot::irc_data::packet::parse(*received);
        for (auto packet : packets) {
          auto &&command = std::get_if<std::string>(&packet.command);
          if (command) {
            if (*command == "ERROR") {
              std::cerr << "Disconnected: "
                        << packet.trailing_param.value_or("???") << std::endl;
              break;
            }
            if (*command == "PING") {
              sock.send(chlorobot::irc_data::packet{
                  .command = "PONG", .trailing_param = packet.trailing_param}
                            .serialize());
            }
          }
          lua.maybe_handle_packet(packet);
        }
      }
    }

    if (!sock.gracefully_disconnected()) {
      throw std::runtime_error{"Socket was not gracefully disconnected!"};
    }

    // Success is assumed when everything finishes with no exceptions
    return EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    return EXIT_FAILURE;
  }
}
