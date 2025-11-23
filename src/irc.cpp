#include "../include/irc.hpp"
using namespace chlorobot;

void irc::send_user_info(const std::string &nick, const std::string &user,
                         const std::string &gecos) {
  tls_socket::socket &sock = tls_socket::socket::get_instance();
  std::cerr << "Requesting nickname '" << nick << "'" << std::endl;
  sock.send(irc_data::packet{.command = "NICK", .params = {nick}}.serialize());
  std::cerr << "Requesting username '" << user << "' and realname '" << gecos
            << "'" << std::endl;
  sock.send(
      irc_data::packet{.command = "USER", .params = {user, "0", "*"}, .trailing_param = gecos}
          .serialize());
}