#pragma once
#include "../protos/chlorobot_rpc.grpc.pb.h"
#include "irc_data.hpp"
#include "main.hpp"
#include "tls_socket.hpp"
namespace chlorobot {
namespace irc {
struct listener {
  void *tag = nullptr;
  grpc::ServerAsyncWriter<ChlorobotPacket> *writer = nullptr;
};

/// @brief Connects to the IRC server by SSL on the specified port with user
/// data
/// @param host The server to connect to
/// @param port The port that we connect to
/// @param user_data The user data to use
/// @param rpc_token The RPC token
void connect(std::string &&, std::string &&, irc_data::user &&, std::string &&);

/// @brief Send an IRC packet
/// @param data The data to send
void send(irc_data::packet &&);

/// @brief Try to receive an IRC packet
/// @return The data received or nothing
std::optional<irc_data::packet> recv();
} // namespace irc
} // namespace chlorobot
