#pragma once
#include "irc_data.hpp"
#include "main.hpp"
#include "tls_socket.hpp"
namespace chlorobot {
namespace irc {
enum class async_state { create, process, finish };

/// @brief Handle Send proto
class request {
  ChlorobotRPC::AsyncService *_service;
  grpc::ServerCompletionQueue *_completion_queue;
  grpc::ServerContext _context;

  ChlorobotRequest _request;
  ChlorobotAcknowledgement _acknowledgement;

  grpc::ServerAsyncResponseWriter<ChlorobotAcknowledgement> _responder;

  async_state _state = async_state::create;
public:
  void proceed();
  request(ChlorobotRPC::AsyncService *, grpc::ServerCompletionQueue *);
};

/// @brief Handle Listen proto
class authentication {
  ChlorobotRPC::AsyncService *_service;
  grpc::ServerCompletionQueue *_completion_queue;
  grpc::ServerContext _context;

  ChlorobotAuthentication _authentication;
  // ChlorobotAcknowledgement _acknowledgement;

  grpc::ServerAsyncWriter<ChlorobotPacket> _responder;

  async_state _state = async_state::create;
public:
  void proceed();
  void broadcast(const ChlorobotPacket &);
  authentication(ChlorobotRPC::AsyncService *, grpc::ServerCompletionQueue *);
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
