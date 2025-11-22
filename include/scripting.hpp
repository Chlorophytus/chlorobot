#pragma once
#include "irc_data.hpp"
#include "main.hpp"
namespace chlorobot {
namespace scripting {
namespace calls {
int reload(lua_State *);
int core_version(lua_State *);
int send(lua_State *);
int stop(lua_State *);
int log_raw(lua_State *);
} // namespace calls

using L_ptr = std::unique_ptr<lua_State, std::function<void(lua_State *)>>;
class engine {
  L_ptr _L{nullptr, [](lua_State *l) {
             if (l) {
               lua_close(l);
             }
           }};

  engine() {
    _L.reset(luaL_newstate());
    luaL_openlibs(_L.get());
    
    // Initialize "Chlorobot" table
    lua_newtable(_L.get());
    lua_pushcfunction(_L.get(), scripting::calls::reload);
    lua_setfield(_L.get(), 1, "reload");
    lua_pushcfunction(_L.get(), scripting::calls::core_version);
    lua_setfield(_L.get(), 1, "core_version");
    lua_pushcfunction(_L.get(), scripting::calls::send);
    lua_setfield(_L.get(), 1, "send");
    lua_pushcfunction(_L.get(), scripting::calls::stop);
    lua_setfield(_L.get(), 1, "stop");
    lua_pushcfunction(_L.get(), scripting::calls::log_raw);
    lua_setfield(_L.get(), 1, "log_raw");
    lua_setglobal(_L.get(), "chlorobot");

    reload_scripts();
  }
  engine(const engine &) = delete;
  engine(engine &&) = delete;
  engine &operator=(const engine &) = delete;
  engine &operator=(engine &&) = delete;

public:
  static engine &get_instance() {
    static engine instance;
    return instance;
  }
  void start();
  int reload_scripts();
  void maybe_handle_packet(const std::optional<irc_data::packet> &);
};
} // namespace scripting
} // namespace chlorobot
