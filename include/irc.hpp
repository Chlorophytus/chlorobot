#pragma once
#include "main.hpp"
#include "tls_socket.hpp"
namespace chlorobot {
namespace irc {
/// @brief Lua scripting calls
namespace scripting {
int send(lua_State *);
int my_username(lua_State *);
int my_owner(lua_State *);
int stop(lua_State *);
int version(lua_State *);
}

/// @brief User identification data sent to the IRC daemon
struct user_data {
  std::string nickname;  // Nickname shown when sending chat messages
  std::string ident;     // Ident field (will be shown to left of hostmask)
  std::string real_name; // Real name shown to whoever does a WHOIS on you

  std::string sasl_account;  // SASL account username
  std::string sasl_password; // SASL password

  std::string owner; // Bot owner username
};

/// @brief Parsed IRC message data
struct message_data {
  std::optional<std::string> prefix = std::nullopt; // The first chunk
  std::variant<U16, std::string> command; // A command numeric or string
  std::vector<std::string> params{};        // Space-separated parameters
  std::optional<std::string> trailing_param = std::nullopt; // The last chunk

  static std::vector<message_data> parse(const std::string);

  const std::string serialize() const;
};

struct socket_ssl {
  const irc::user_data data;
  lua_State *L = nullptr;
  bool running = true;

  /// @brief Connects to the IRC server by SSL on the specified port with user
  /// data
  /// @param host The server to connect to
  /// @param port The port that we connect to
  /// @param user_data The user data to use
  socket_ssl(std::string &&, std::string &&, user_data &&);
  ~socket_ssl();

  void send(message_data &&);
  std::optional<message_data> recv();
};
} // namespace irc
} // namespace chlorobot
