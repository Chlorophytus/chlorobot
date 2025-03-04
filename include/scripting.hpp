#pragma once
#include "main.hpp"
#include "irc_data.hpp"
namespace chlorobot {
namespace scripting {
  /// @brief initializes the Lua scripting engine's state
void start(std::filesystem::path &&);
  /// @brief (re)loads the start script
int load_startup();
/// @brief handles one runloop tick
void maybe_handle_packet(const std::optional<irc_data::packet> &);
/// @brief destroys the Lua scripting engine's state
void stop();

namespace calls {
  int reload(lua_State *);
  int core_version(lua_State *);
  int send(lua_State *);
  int stop(lua_State *);
}
}
} // namespace chlorobot
