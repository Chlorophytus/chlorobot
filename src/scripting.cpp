#include "../include/scripting.hpp"
#include "../include/tls_socket.hpp"
#include "lua.h"
using namespace chlorobot;

using L_sptr = std::unique_ptr<lua_State, std::function<void(lua_State *)>>;
static L_sptr L;
static std::filesystem::path root;

void scripting::start(std::filesystem::path &&r) {
  if (!L) {
    root = r;
    L = L_sptr{luaL_newstate(), [](lua_State *l) { lua_close(l); }};
    luaL_openlibs(L.get());

    // Initialize "Chlorobot" table
    lua_newtable(L.get());
    lua_pushcfunction(L.get(), scripting::calls::reload);
    lua_setfield(L.get(), 1, "reload");
    lua_pushcfunction(L.get(), scripting::calls::core_version);
    lua_setfield(L.get(), 1, "core_version");
    lua_pushcfunction(L.get(), scripting::calls::send);
    lua_setfield(L.get(), 1, "send");
    lua_pushcfunction(L.get(), scripting::calls::stop);
    lua_setfield(L.get(), 1, "stop");
    lua_setglobal(L.get(), "chlorobot");

    scripting::load_startup();
  } else {
    throw std::runtime_error{"Lua state already existed"};
  }
}

int scripting::load_startup() {
  std::cerr << "Loading startup Lua script" << std::endl;
  auto ret = luaL_dofile(L.get(), (root / "priv" / "init.lua").c_str());

  if (ret != 0) {
    std::cerr << "Startup Lua script error: " << lua_tostring(L.get(), -1)
              << std::endl;
  }
  return ret;
}

void scripting::maybe_handle_packet(
    const std::optional<irc_data::packet> &maybe_packet) {
  lua_getglobal(L.get(), "chlorobot");
  lua_getfield(L.get(), 1, "tick");
  if (maybe_packet) {
    lua_newtable(L.get());
    // prefix
    if (maybe_packet->prefix) {
      lua_pushstring(L.get(), maybe_packet->prefix->c_str());
    } else {
      lua_pushnil(L.get());
    }
    lua_setfield(L.get(), -2, "prefix");

    // command
    if (maybe_packet->command.index() == 0) {
      lua_pushinteger(L.get(), std::get<U32>(maybe_packet->command));
    } else {
      lua_pushstring(L.get(),
                     std::get<std::string>(maybe_packet->command).c_str());
    }
    lua_setfield(L.get(), -2, "command");

    // params
    U64 i = 1;
    lua_newtable(L.get());
    for (auto &&param : maybe_packet->params) {
      lua_pushnumber(L.get(), i++);
      lua_pushstring(L.get(), param.c_str());
      lua_settable(L.get(), -3);
    }
    lua_setfield(L.get(), -2, "params");

    // trailing
    if (maybe_packet->trailing_param) {
      lua_pushstring(L.get(), maybe_packet->trailing_param->c_str());
    } else {
      lua_pushnil(L.get());
    }
    lua_setfield(L.get(), -2, "trailing_param");
  } else {
    lua_pushnil(L.get());
  }

  auto ret = lua_pcall(L.get(), 1, 0, 0);
  if (ret != 0) {
    std::cerr << "Startup script runloop error: " << lua_tostring(L.get(), -1)
              << std::endl;
    throw std::runtime_error{"Startup script runloop error"};
  }
}

void scripting::stop() { L = nullptr; }

int scripting::calls::reload(lua_State *l) {
  std::cerr << "Startup script requested reload" << std::endl;
  int ret = scripting::load_startup();
  lua_pushinteger(l, ret);
  return 1;
}

int scripting::calls::core_version(lua_State *l) {
  lua_newtable(l);
  lua_pushinteger(l, chlorobot_VMAJOR);
  lua_setfield(l, 1, "major");
  lua_pushinteger(l, chlorobot_VMINOR);
  lua_setfield(l, 1, "minor");
  lua_pushinteger(l, chlorobot_VPATCH);
  lua_setfield(l, 1, "patch");
  lua_pushinteger(l, chlorobot_VTWEAK);
  lua_setfield(l, 1, "revision");
  return 1;
}

int scripting::calls::send(lua_State *l) {
  if (lua_istable(l, 1)) {
    // prefix
    decltype(irc_data::packet::prefix) prefix = std::nullopt;
    lua_getfield(l, 1, "prefix");
    if (lua_isstring(l, -1)) {
      prefix = lua_tostring(l, -1);
    }
    lua_pop(l, 1);

    // command
    decltype(irc_data::packet::command) command{};
    lua_getfield(l, 1, "command");
    if (lua_isnumber(l, -1)) {
      command = static_cast<U32>(lua_tointeger(l, -1));
    } else if (lua_isstring(l, -1)) {
      command = lua_tostring(l, -1);
    }
    lua_pop(l, 1);

    // trailing
    decltype(irc_data::packet::trailing_param) trailing = std::nullopt;
    lua_getfield(l, 1, "trailing_param");
    if (lua_isstring(l, -1)) {
      trailing = lua_tostring(l, -1);
    }
    lua_pop(l, 1);

    // params table
    decltype(irc_data::packet::params) params{};
    lua_getfield(l, 1, "params");
    if (lua_istable(l, -1)) {
      lua_pushnil(l);
      while (lua_next(l, -2) != 0) {
        params.emplace_back(lua_tostring(l, -1));
        lua_pop(l, 1);
      }
    }
    lua_pop(l, 1);

    tls_socket::send(irc_data::packet{.prefix = prefix,
                                      .command = command,
                                      .params = params,
                                      .trailing_param = trailing}
                         .serialize());
  }

  return 0;
}

int scripting::calls::stop(lua_State *l) {
  std::cerr << "Startup script requested quit" << std::endl;
  tls_socket::send(
      irc_data::packet{.command = "QUIT", .trailing_param = "Chlorobot"}
          .serialize());

  return 0;
}
