#pragma once
#include "main.hpp"
namespace chlorobot {
namespace tls_socket {
enum class poll_state {
  ok,
  retry,
  end_of_stream,
  error,
};

/// @brief Connects as a client
/// @param host The host to connect to
/// @param port The port to use
void connect(const std::string, const std::string);

/// @brief Sends information to the server by the socket
/// @param message What to send to the server
void send(const std::string);

/// @brief Receives information from the server by the socket
/// @return message The message received
std::optional<std::string> recv();

/// @brief Disconnects the client
void disconnect();

/// @brief Returns if the TLS client encountered an end-of-file
bool is_eof();
} // namespace tls_socket
} // namespace chlorobot
