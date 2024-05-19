#include "../include/irc.hpp"
using namespace chlorobot;

constexpr static auto trigger_rate = std::chrono::milliseconds(10);
constexpr static bool debug = true;

int irc::scripting::stop(lua_State *L) {
  lua_getfield(L, 1, "context");
  auto S = reinterpret_cast<irc::socket_ssl *>(lua_touserdata(L, -1));
  S->running = false;

  return 0;
}

int irc::scripting::my_username(lua_State *L) {
  lua_getfield(L, 1, "context");
  auto S = reinterpret_cast<irc::socket_ssl *>(lua_touserdata(L, -1));
  lua_pushstring(L, S->data.nickname.c_str());

  return 1;
}

int irc::scripting::my_owner(lua_State *L) {
  lua_getfield(L, 1, "context");
  auto S = reinterpret_cast<irc::socket_ssl *>(lua_touserdata(L, -1));
  lua_pushstring(L, S->data.owner.c_str());

  return 1;
}

int irc::scripting::send(lua_State *L) {
  lua_getfield(L, 1, "context");
  auto S = reinterpret_cast<irc::socket_ssl *>(lua_touserdata(L, -1));

  decltype(irc::message_data::prefix) prefix = std::nullopt;
  decltype(irc::message_data::command) command;
  decltype(irc::message_data::params) params{};
  decltype(irc::message_data::trailing_param) trailing_param = std::nullopt;

  lua_getfield(L, 2, "prefix");
  if (lua_isstring(L, -1)) {
    prefix.emplace(lua_tostring(L, -1));
  }
  lua_pop(L, 1);

  lua_getfield(L, 2, "command");
  if (lua_isinteger(L, -1)) {
    command = static_cast<U16>(lua_tointeger(L, -1));
  } else {
    command = std::string{lua_tostring(L, -1)};
  }
  lua_pop(L, 1);

  lua_getfield(L, 2, "trailing_parameter");
  if (lua_isstring(L, -1)) {
    trailing_param.emplace(lua_tostring(L, -1));
  }
  lua_pop(L, 1);

  lua_getfield(L, 2, "parameters");
  if (!lua_isnil(L, -1)) {
    lua_pushnil(L);
    while (lua_next(L, -2)) {
      params.emplace_back(lua_tostring(L, -1));
      lua_pop(L, 1);
    }
  }

  tls_socket::send(irc::message_data{.prefix = prefix,
                                     .command = command,
                                     .params = params,
                                     .trailing_param = trailing_param}
                       .serialize());

  return 0;
}

int irc::scripting::version(lua_State *L) {
  lua_pushstring(L, chlorobot_VSTRING_FULL);
  return 1;
}

irc::socket_ssl::socket_ssl(std::string &&host, std::string &&port,
                            user_data &&_data)
    : data{_data} {
  tls_socket::connect(host, port);

  std::cerr << "--- Connected ---" << std::endl;

  // Request SASL
  std::cerr << "Requesting SASL" << std::endl;
  tls_socket::send(
      irc::message_data{.command = "CAP", .params = {"LS"}}.serialize());

  // Nickname
  std::cerr << "Sending nickname" << std::endl;
  tls_socket::send(
      irc::message_data{.command = "NICK", .params = {data.nickname}}
          .serialize());

  // Username
  std::cerr << "Sending username" << std::endl;
  tls_socket::send(irc::message_data{.command = "USER",
                                     .params = {data.ident, "0", "*"},
                                     .trailing_param = data.real_name}
                       .serialize());

  // CAP SASL Acknowledge waiter
  std::cerr << "Waiting for SASL listing from server" << std::endl;
  bool waiting = true;
  while (waiting) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = message_data::parse(*recv);
      for (auto &&packet : packets) {
        const auto command = packet.command;
        if (command.index() == 1) {
          if (std::get<std::string>(command) == "CAP") {
            std::string trailing_param = packet.trailing_param.value_or("");
            for (auto &&cap : std::views::split(trailing_param, ' ')) {
              if (std::string{cap.begin(), cap.end()} == "sasl") {
                tls_socket::send(irc::message_data{.command = "CAP",
                                                   .params = {"REQ"},
                                                   .trailing_param = "sasl"}
                                     .serialize());
                waiting = false;
                break;
              }
            }
            if (waiting) {
              throw std::runtime_error{
                  "This IRC server does not support SASL!"};
            }
          }
        }
      }
    }
  }

  std::cerr << "Waiting for SASL acknowledge from server" << std::endl;
  waiting = true;
  while (waiting) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = message_data::parse(*recv);
      for (auto &&packet : packets) {
        const auto command = packet.command;
        if (command.index() == 1 && std::get<std::string>(command) == "CAP" &&
            packet.params.at(1) == "ACK" &&
            packet.trailing_param.value_or("") == "sasl") {
          tls_socket::send(
              irc::message_data{.command = "AUTHENTICATE", .params = {"PLAIN"}}
                  .serialize());
          waiting = false;
          break;
        }
      }
    }
  }

  std::cerr << "Waiting for SASL authentication initiation from server"
            << std::endl;
  waiting = true;
  while (waiting) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = message_data::parse(*recv);
      for (auto &&packet : packets) {
        const auto command = packet.command;
        if (command.index() == 1) {
          if (std::get<std::string>(command) == "AUTHENTICATE" &&
              packet.params.at(0) == "+") {
            // this is a bit tricky
            waiting = false;
            char base64_in[400]{0};
            char base64_out[400]{0};
            // authcid
            U32 i = 0;
            for (char in : data.sasl_account) {
              base64_in[i] = in;
              i++;
            }
            base64_in[i++] = 0;
            // authzid
            for (char in : data.sasl_account) {
              base64_in[i] = in;
              i++;
            }
            base64_in[i++] = 0;
            // passwd
            for (char in : data.sasl_password) {
              base64_in[i] = in;
              i++;
            }

            boost::beast::detail::base64::encode(base64_out, base64_in, i);

            tls_socket::send(irc::message_data{
                .command = "AUTHENTICATE", .params = {std::string{base64_out}}}
                                 .serialize());
            tls_socket::send(
                irc::message_data{.command = "CAP", .params = {"END"}}
                    .serialize());
            break;
          }
        }
      }
    }
  }

  std::cerr << "Starting Lua" << std::endl;
  L = luaL_newstate();
  luaL_openlibs(L);

  lua_newtable(L);

  lua_pushcfunction(L, irc::scripting::stop);
  lua_setfield(L, 1, "stop");

  lua_pushcfunction(L, irc::scripting::send);
  lua_setfield(L, 1, "send");

  lua_pushcfunction(L, irc::scripting::version);
  lua_setfield(L, 1, "version");

  lua_pushcfunction(L, irc::scripting::my_username);
  lua_setfield(L, 1, "my_username");

  lua_pushcfunction(L, irc::scripting::my_owner);
  lua_setfield(L, 1, "my_owner");

  lua_pushlightuserdata(L, this);
  lua_setfield(L, 1, "context");

  lua_setglobal(L, "chlorobot");

  luaL_dofile(L, "priv/init.lua");

  while (running) {
    const auto recv = tls_socket::recv();
    if (recv) {
      const auto packets = message_data::parse(*recv);

      for (auto &&packet : packets) {

        if (packet.command.index() == 1 &&
            std::get<std::string>(packet.command) == "ERROR") {
          running = false;
          break;
        }

        lua_getglobal(L, "on_recv");
        lua_newtable(L);

        lua_pushstring(L, "prefix");
        if (packet.prefix) {
          lua_pushstring(L, packet.prefix->c_str());
        } else {
          lua_pushnil(L);
        }
        lua_settable(L, -3);

        lua_pushstring(L, "command");
        if (packet.command.index() == 0) {
          lua_pushinteger(
              L, static_cast<lua_Integer>(std::get<U16>(packet.command)));
        } else {
          lua_pushstring(L, std::get<std::string>(packet.command).c_str());
        }
        lua_settable(L, -3);

        lua_pushstring(L, "parameters");
        lua_newtable(L);
        U64 i = 1;
        for (auto &&param : packet.params) {
          lua_pushnumber(L, i++);
          lua_pushstring(L, param.c_str());
          lua_settable(L, -3);
        }
        lua_settable(L, -3);

        lua_pushstring(L, "trailing_parameter");
        if (packet.trailing_param) {
          lua_pushstring(L, packet.trailing_param->c_str());
        } else {
          lua_pushnil(L);
        }
        lua_settable(L, -3);
        lua_pcall(L, 1, 0, 0);
      }
    }
  }
  lua_close(L);
  irc::tls_socket::send(irc::message_data{
      .command = "QUIT", .trailing_param = "Run loop has halted"}
                            .serialize());
}

irc::socket_ssl::~socket_ssl() {
  std::cerr << "Disconnecting socket" << std::endl;
  irc::tls_socket::disconnect();
}

std::vector<irc::message_data>
irc::message_data::parse(const std::string message) {
  auto msgs = message;
  std::vector<irc::message_data> rets{};

  for (auto &&msg_raw : std::views::split(msgs, '\n')) {
    auto msg = std::string{msg_raw.begin(), msg_raw.end()};
    if (msg.back() == '\r') {
      msg.pop_back();
    }
    if(msg.empty()) {
      continue;
    }
    if (debug) {
      std::cerr << "[RECV] " << std::string{msg.begin(), msg.end()}
                << std::endl;
    }
    U64 i = 0;

    decltype(irc::message_data::prefix) prefix = std::nullopt;
    decltype(irc::message_data::command) command{};
    decltype(irc::message_data::params) params{};
    decltype(irc::message_data::trailing_param) trailing_param = std::nullopt;

    for (auto &&split : std::views::split(msg, ' ')) {
      if (trailing_param) {
        *trailing_param += ' ';
        *trailing_param += std::string{split.begin(), split.end()};
      } else {
        switch (i) {
        case 0: {
          if (split[0] == ':') {
            prefix.emplace(std::string{split.begin(), split.end()}.substr(1));
          } else {
            try {
              // We're using Boost anyway
              command.emplace<U16>(boost::lexical_cast<U16>(
                  std::string{split.begin(), split.end()}));
            } catch (boost::bad_lexical_cast _) {
              command.emplace<std::string>(
                  std::string{split.begin(), split.end()});
            }
          }
          break;
        }
        case 1: {
          if (prefix) {
            // Command now
            try {
              // We're using Boost anyway
              command.emplace<U16>(boost::lexical_cast<U16>(
                  std::string{split.begin(), split.end()}));
            } catch (boost::bad_lexical_cast _) {
              command.emplace<std::string>(
                  std::string{split.begin(), split.end()});
            }
          } else if (split[0] == ':') {
            // Suffix now
            trailing_param = std::string{split.begin(), split.end()}.substr(1);
          } else {
            // Space-separated parameters now
            params.emplace_back(std::string{split.begin(), split.end()});
          }
          break;
        }
        default: {
          if (split[0] == ':') {
            // Suffix now
            trailing_param = std::string{split.begin(), split.end()}.substr(1);
          } else {
            // Space-separated parameters now
            params.emplace_back(std::string{split.begin(), split.end()});
          }
          break;
        }
        }
        i++;
      }
    }
    rets.emplace_back(irc::message_data{.prefix = prefix,
                                        .command = command,
                                        .params = params,
                                        .trailing_param = trailing_param});
  }
  return rets;
}

const std::string irc::message_data::serialize() const {
  std::string message = "";
  if (prefix) {
    message += ":" + *prefix + " ";
  }
  if (command.index() == 0) {
    message += std::to_string(std::get<U16>(command));
  } else {
    message += std::get<std::string>(command);
  }
  for (const std::string &param : params) {
    message += " " + param;
  }
  if (trailing_param) {
    message += " :" + *trailing_param;
  }
  if (debug) {
    std::cerr << "[SEND] " << message << std::endl;
  }
  return message + "\r\n";
}