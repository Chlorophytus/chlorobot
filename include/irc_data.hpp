#pragma once
#include "main.hpp"
namespace chlorobot {
namespace irc_data {
/// @brief Parsed IRC packet data
struct packet {
  std::optional<std::string> prefix = std::nullopt; // The first chunk
  std::variant<U32, std::string> command; // A command numeric or string
  std::vector<std::string> params{};      // Space-separated parameters
  std::optional<std::string> trailing_param = std::nullopt; // The last chunk

  /// @brief Parses a string into an IRC packet
  /// @param unparsed_data The unparsed IRC data
  /// @return A parsed IRC packet data structure
  static std::vector<packet> parse(const std::string);

  /// @brief Serializes an IRC packet into a string
  /// @return A serialized IRC packet string
  const std::string serialize() const;
};
} // namespace irc_data
} // namespace chlorobot
