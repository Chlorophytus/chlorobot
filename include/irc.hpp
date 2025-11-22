#pragma once
#include "irc_data.hpp"
#include "main.hpp"
#include "tls_socket.hpp"
namespace chlorobot {
namespace irc {
/// @brief Sends NICK and USER to IRC
/// @param nick the IRC nickname
/// @param user the IRC username
/// @param gecos the IRC real name
void send_user_info(const std::string &nick, const std::string &user,
                    const std::string &gecos);
} // namespace irc
} // namespace chlorobot
