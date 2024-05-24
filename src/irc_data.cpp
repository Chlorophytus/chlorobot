#include "../include/irc_data.hpp"
using namespace chlorobot;

decltype(irc_data::packet::command) parse_command_variant(std::string &&str) {
  U32 numeric = 0;
  if (str.length() == 3) {
    // Hundreds
    if (std::isdigit(str[0])) {
      switch (str[0]) {
      default: {
        break;
      }
      case '1': {
        numeric += 100;
        break;
      }
      case '2': {
        numeric += 200;
        break;
      }
      case '3': {
        numeric += 300;
        break;
      }
      case '4': {
        numeric += 400;
        break;
      }
      case '5': {
        numeric += 500;
        break;
      }
      case '6': {
        numeric += 600;
        break;
      }
      case '7': {
        numeric += 700;
        break;
      }
      case '8': {
        numeric += 800;
        break;
      }
      case '9': {
        numeric += 900;
        break;
      }
      }
    } else {
      return str;
    }
    // Tens
    if (std::isdigit(str[1])) {
      switch (str[1]) {
      default: {
        break;
      }
      case '1': {
        numeric += 10;
        break;
      }
      case '2': {
        numeric += 20;
        break;
      }
      case '3': {
        numeric += 30;
        break;
      }
      case '4': {
        numeric += 40;
        break;
      }
      case '5': {
        numeric += 50;
        break;
      }
      case '6': {
        numeric += 60;
        break;
      }
      case '7': {
        numeric += 70;
        break;
      }
      case '8': {
        numeric += 80;
        break;
      }
      case '9': {
        numeric += 90;
        break;
      }
      }
    } else {
      return str;
    }
    // Ones
    if (std::isdigit(str[2])) {
      switch (str[2]) {
      default: {
        break;
      }
      case '1': {
        numeric += 1;
        break;
      }
      case '2': {
        numeric += 2;
        break;
      }
      case '3': {
        numeric += 3;
        break;
      }
      case '4': {
        numeric += 4;
        break;
      }
      case '5': {
        numeric += 5;
        break;
      }
      case '6': {
        numeric += 6;
        break;
      }
      case '7': {
        numeric += 7;
        break;
      }
      case '8': {
        numeric += 8;
        break;
      }
      case '9': {
        numeric += 9;
        break;
      }
      }
    } else {
      return str;
    }
    return numeric;
  }
  return str;
}

std::vector<irc_data::packet>
irc_data::packet::parse(const std::string message) {
  auto msgs = message;
  std::vector<irc_data::packet> rets{};

  for (auto &&msg_raw : std::views::split(msgs, '\n')) {
    auto msg = std::string{msg_raw.begin(), msg_raw.end()};
    if (msg.back() == '\r') {
      msg.pop_back();
    }
    if (msg.empty()) {
      continue;
    }
    U64 i = 0;

    decltype(irc_data::packet::prefix) prefix = std::nullopt;
    decltype(irc_data::packet::command) command{};
    decltype(irc_data::packet::params) params{};
    decltype(irc_data::packet::trailing_param) trailing_param = std::nullopt;

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
            command =
                parse_command_variant(std::string{split.begin(), split.end()});
          }
          break;
        }
        case 1: {
          if (prefix) {
            command =
                parse_command_variant(std::string{split.begin(), split.end()});
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
#ifdef chlorobot_DEBUG
    std::cerr
        << "[" << std::chrono::system_clock::now() << "] [RECV] " << msg
        << std::endl;
#endif
    rets.emplace_back(irc_data::packet{.prefix = prefix,
                                       .command = command,
                                       .params = params,
                                       .trailing_param = trailing_param});
  }
  return rets;
}

const std::string irc_data::packet::serialize() const {
  std::string message = "";
  if (prefix) {
    message += ":" + *prefix + " ";
  }
  if (command.index() == 0) {
    message += std::to_string(std::get<U32>(command));
  } else {
    message += std::get<std::string>(command);
  }
  for (const std::string &param : params) {
    message += " " + param;
  }
  if (trailing_param) {
    message += " :" + *trailing_param;
  }
#ifdef chlorobot_DEBUG
  std::cerr
      << "[" << std::chrono::system_clock::now() << "] [SEND] " << message
      << std::endl;
#endif
  return message + "\r\n";
}