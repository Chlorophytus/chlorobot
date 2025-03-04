#pragma once
#include "irc_data.hpp"
#include "main.hpp"
#include "tls_socket.hpp"
namespace chlorobot {
namespace irc {
/// @brief Connects to the IRC server by SSL on the specified port with user
/// data
/// @param host The server to connect to
/// @param port The port that we connect to
/// @param user_data The user data to use
void connect(std::string &&, std::string &&, irc_data::user &&);
} // namespace irc
} // namespace chlorobot
