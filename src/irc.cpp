#include "../include/irc.hpp"
using namespace chlorobot;

constexpr static auto sasl_timeout = std::chrono::seconds(10);

irc::socket_ssl::socket_ssl(std::string &&host, std::string &&port,
                            user_data &&_data)
    : data{_data}, stream{context, ssl_context} {
  std::cerr << "Resolving '" << host << "'..." << std::endl;
  boost::asio::ip::tcp::resolver resolver(context);
  auto endpoints = resolver.resolve(host, port);

  std::cerr << "Connecting..." << std::endl;
  boost::asio::connect(stream.lowest_layer(), endpoints);

  std::cerr << "SSL Handshake..." << std::endl;
  stream.handshake(decltype(stream)::handshake_type::client);

  std::cerr << "--- Connected ---" << std::endl;

  boost::asio::streambuf writer_buffer;
  std::ostream writer_stream(&writer_buffer);
  // Request SASL
  std::cerr << "Requesting SASL" << std::endl;
  writer_stream << "CAP LS\r\n";
  boost::asio::write(stream, writer_buffer);

  // Nickname
  std::cerr << "Sending nickname" << std::endl;
  writer_stream << "NICK " << data.nickname << "\r\n";
  boost::asio::write(stream, writer_buffer);

  // Username
  std::cerr << "Sending username" << std::endl;
  writer_stream << "USER " << data.ident << " 0 0 :" << data.real_name
                << "\r\n";
  boost::asio::write(stream, writer_buffer);

  std::cerr << "Waiting for SASL acknowledge" << std::endl;
  bool sasl_still_waiting = true;
  auto t0 = std::chrono::steady_clock::now();
  auto t1 = std::chrono::steady_clock::now();
  // TODO: State machine for authenticating???
  while (sasl_still_waiting) {
    boost::asio::streambuf reader_buffer{};
    const auto sasl_bytes_read =
        boost::asio::read_until(stream, reader_buffer, "\r\n");
    if (sasl_bytes_read > 0) {
      std::string reader;
      std::istream reader_stream(&reader_buffer);
      std::getline(reader_stream, reader);

      const auto message = irc::message_data::parse(reader);

      if (message.prefix) {
        std::cerr << "[Prefix: " << *message.prefix << "] ";
      }
      if (message.command.index() == 0) {
        std::cerr << "[Command U16: " << std::get<U16>(message.command) << "] ";
      } else {
        std::cerr << "[Command String: "
                  << std::get<std::string>(message.command) << "] ";
      }
      std::stringstream params;
      std::copy(message.params.begin(), message.params.end(),
                std::ostream_iterator<std::string>(params, ", "));
      std::cerr << "[SpacedParams: (" << params.str() << ")] ";
      if (message.trailing_param) {
        std::cerr << "[TrailingParam: " << *(message.trailing_param) << "] ";
      }
      std::cerr << std::endl;
    }
    t1 = std::chrono::steady_clock::now();
    if ((t1 - t0) > sasl_timeout) {
      throw std::runtime_error{"Couldn't authenticate on time"};
    }
  }
}

irc::socket_ssl::~socket_ssl() {
  std::cerr << "Disconnecting socket" << std::endl;
  stream.shutdown();
}

irc::message_data irc::message_data::parse(const std::string &message) {
  U64 i = 0;
  std::string clipped = message;
  clipped.pop_back();
  decltype(irc::message_data::prefix) prefix = std::nullopt;
  decltype(irc::message_data::command) command{};
  decltype(irc::message_data::params) params{};
  decltype(irc::message_data::trailing_param) trailing_param = std::nullopt;

  for (const auto &split : std::views::split(clipped, ' ')) {
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

  return irc::message_data{.prefix = prefix,
                           .command = command,
                           .params = params,
                           .trailing_param = trailing_param};
}
