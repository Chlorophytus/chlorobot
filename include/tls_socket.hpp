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
using SSL_CTX_ptr = std::unique_ptr<SSL_CTX, std::function<void(SSL_CTX *)>>;
using SSL_ptr = std::unique_ptr<SSL, std::function<void(SSL *)>>;
using BIO_ptr = std::unique_ptr<BIO, std::function<void(BIO *)>>;
using SSLCHAR_ptr = std::unique_ptr<char, std::function<void(char *)>>;

constexpr static auto max_buffer_size = 4096;
constexpr static auto io_timeout_microseconds = 50'000;

/// @brief Singleton for a SSL socket
class socket {
  socket() = default;
  socket(const socket &) = delete;
  socket(socket &&) = delete;
  socket &operator=(const socket &) = delete;
  socket &operator=(socket &&) = delete;

  SSL_CTX_ptr _context{nullptr, [](SSL_CTX *ptr) {
                         if (ptr) {
                           SSL_CTX_free(ptr);
                         }
                       }};
  SSL_ptr _ssl{nullptr, [](SSL *ptr) {
                 if (ptr) {
                   SSL_free(ptr);
                 }
               }};
  // NOTE: the SSL BIO is freed on its own...
  BIO *_bio = nullptr;
  int _sock = -1;
  bool _gracefully_disconnected = false;

  void _initialize_bio(const std::string &host, const std::string &port);
  poll_state _handle_data(int resource);

public:
  /// @brief Gets the socket singleton
  /// @return A reference to the socket singleton
  static socket &get_instance() {
    static socket instance;
    return instance;
  }

  /// @brief Connects as a client
  /// @param host The host to connect to
  /// @param port The port to use
  void connect(const std::string &host, const std::string &port);

  /// @brief Sends information to the server by the socket
  /// @param message What to send to the server
  void send(const std::string &);

  /// @brief Receives information from the server by the socket
  /// @return message The message received
  std::optional<std::string> recv();

  /// @brief Deletes the socket, disconnecting it if connected
  void disconnect();

  /// @brief Returns true if the socket is connected
  operator bool();

  /// @brief Returns whether or not the connection was intentionally
  /// disconnected
  /// @return True if it was intentionally disconnected by the client
  bool gracefully_disconnected() const;
};
} // namespace tls_socket
} // namespace chlorobot
