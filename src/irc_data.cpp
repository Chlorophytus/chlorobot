#include "../include/irc_data.hpp"
using namespace chlorobot;
std::variant<U32, std::string> parse_command_variant(const std::string str) {
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
  std::istringstream msgs;
  msgs.str(message);

  std::vector<irc_data::packet> rets{};

  for (std::string line; std::getline(msgs, line, '\n');) {
    auto msg = line;

    if (msg.back() == '\r') {
      msg.pop_back();
    }

    if (!msg.empty()) {
      decltype(irc_data::packet::prefix) prefix = std::nullopt;
      decltype(irc_data::packet::command) command{};
      decltype(irc_data::packet::params) params{};
      decltype(irc_data::packet::trailing_param) trailing_param = std::nullopt;

      auto offset = 0;

      if (msg.front() == ':') {
        // Has prefix
        const auto prefix_separator = msg.find(' ');
        prefix = msg.substr(1, prefix_separator - 1);
        offset = prefix_separator;
        std::cerr << "prefix: '" << *prefix << "'" << std::endl;
      }

      auto placement = 0;
      auto looping = true;
      const auto msg_size = msg.size();

      do {
        switch (placement) {
        case 0: {
          // Commands are always our first parameter
          const auto command_separator = msg.find(' ', offset);
          if (command_separator == std::string::npos) {
            command = parse_command_variant(msg.substr(offset));

            looping = false;
          } else {
            command = parse_command_variant(msg.substr(offset, command_separator - 1));
            offset += command_separator;
          }
          if(command.index() == 0) {
            std::cerr << "command (U32): '" << std::get<U32>(command) << "'" << std::endl;
          } else {
            std::cerr << "command (string): '" << std::get<std::string>(command) << "'" << std::endl;
          }
          break;
        }
        default: {
          if (msg.at(offset) == ':') {
            // Has trailing parameter
            trailing_param = msg.substr(offset + 1);
            std::cerr << "trailing: '" << *trailing_param << "'" << std::endl;

            looping = false;
          } else {
            // Not a trailing parameter, space-separated
            const auto param_separator = msg.find(' ', offset);
            if (param_separator != std::string::npos) {
              params.emplace_back(msg.substr(offset, param_separator - 1));
              offset += param_separator;
            } else {
              params.emplace_back(msg.substr(offset));

              looping = false;
            }
            std::cerr << "param: '" << params.back() << "'" << std::endl;
          }
          break;
        }
        }

        if(offset >= msg_size) {
          looping = false;
        }

        placement++;
      } while (looping);

      rets.emplace_back(irc_data::packet{.prefix = prefix,
                                         .command = command,
                                         .params = params,
                                         .trailing_param = trailing_param});
    }
  }
  return rets;
}

const std::string irc_data::packet::serialize() const {
  std::string message = "";
  if (prefix) {
    message += ":" + *prefix + " ";
  }
  if (command.index() == 0) {
    std::stringstream padder;
    padder << std::setw(3) << std::setfill('0') << std::get<U32>(command);
    message += padder.str();
  } else {
    message += std::get<std::string>(command);
  }
  for (const std::string &param : params) {
    message += " " + param;
  }
  if (trailing_param) {
    message += " :" + *trailing_param;
  }
  return message + "\r\n";
}
