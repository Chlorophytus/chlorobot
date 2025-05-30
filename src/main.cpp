#include "../include/main.hpp"
#include "../include/irc.hpp"
#include "../include/irc_data.hpp"

int main(int argc, char **argv) {
  // Fail safe
  int status = EXIT_FAILURE;
  try {
    std::cerr << "Chlorobot " << chlorobot_VSTRING_FULL << std::endl;

    // This should be stored in a .env file!
    const std::string nickname = std::getenv("CHLOROBOT_NICKNAME");
    const std::string ident = std::getenv("CHLOROBOT_IDENT");
    const std::string real_name = std::getenv("CHLOROBOT_REALNAME");
    const std::string sasl_username = std::getenv("CHLOROBOT_SASL_USERNAME");
    const std::string sasl_password = std::getenv("CHLOROBOT_SASL_PASSWORD");

    chlorobot::irc::connect(
        std::string{std::getenv("CHLOROBOT_NETWORK_HOST")},
        std::string{std::getenv("CHLOROBOT_NETWORK_PORT")},
        chlorobot::irc_data::user{.nickname = nickname,
                                  .ident = ident,
                                  .real_name = real_name,
                                  .sasl_account = sasl_username,
                                  .sasl_password = sasl_password});

    // Success is assumed when everything finishes with no exceptions
    status = EXIT_SUCCESS;
  } catch (const std::exception &e) {
    std::cerr << "ERROR: " << e.what() << '\n';
  }
  return status;
}
