#include "../include/irc_sasl.hpp"
#include "../include/irc_data.hpp"
#include "../include/tls_socket.hpp"
#include <openssl/evp.h>
#include <ranges>
#include <string_view>
using namespace chlorobot;

bool irc_sasl::has_sasl_capability() {
  tls_socket::socket &sock = tls_socket::socket::get_instance();
  std::cerr << "Requesting server capabilities" << std::endl;
  sock.send(
      irc_data::packet{.command = "CAP", .params = {"LS", "302"}}.serialize());

  std::optional<std::string> raw = std::nullopt;
  std::string sasl_capability = "";
  auto t0 = std::chrono::steady_clock::now();
  while (sasl_capability.empty()) {
    raw = sock.recv();
    if (raw) {
      std::string message = *raw;
      std::vector<irc_data::packet> packets = irc_data::packet::parse(message);

      for (const irc_data::packet &packet : packets) {
        auto &&get_if = std::get_if<std::string>(&packet.command);
        if (get_if && *get_if == "CAP" && packet.params.at(1) == "LS" &&
            packet.trailing_param.has_value()) {
          std::string_view caps = *packet.trailing_param;
          std::string_view capability = "";

          for (auto cap_index = caps.find(' ');
               cap_index != std::string_view::npos;
               cap_index = caps.find(' ')) {
            capability = caps.substr(0, cap_index);
            if (capability.starts_with("sasl")) {
              std::cerr << "Got SASL capability word '" << capability << "'"
                        << std::endl;
              sasl_capability = capability;
            }
            caps.remove_prefix(cap_index + 1);
          }
        }
      }
    }
    auto t1 = std::chrono::steady_clock::now();
    if (t1 > (t0 + irc_sasl::max_capability_time)) {
      std::cerr << "SASL support query timed out" << std::endl;
      return false;
    }
  }
  if (sasl_capability.size() > 4) {
    for (const auto sasl : std::views::split(sasl_capability.substr(5), ',')) {
      if (std::string_view{sasl} == "PLAIN") {
        std::cerr << "Got SASL capability PLAIN" << std::endl;
        return true;
      }
    }

    return false;
  } else {
    std::cerr << "SASL support specified to be empty, proceeding anyway"
              << std::endl;
    return true;
  }
}

void irc_sasl::try_auth(const std::string &account,
                        const std::string &password) {
  tls_socket::socket &sock = tls_socket::socket::get_instance();
  sock.send(irc_data::packet{
      .command = "CAP", .params = {"REQ"}, .trailing_param = "sasl"}
                .serialize());
  std::optional<std::string> raw = std::nullopt;
  irc_sasl::sasl_state state = irc_sasl::sasl_state::wait_for_ack;
  auto t0 = std::chrono::steady_clock::now();

  while (state != irc_sasl::sasl_state::complete) {
    raw = sock.recv();

    if (raw) {
      std::string message = *raw;
      std::vector<irc_data::packet> packets = irc_data::packet::parse(message);

      for (const irc_data::packet &packet : packets) {
        switch (state) {
        case irc_sasl::sasl_state::wait_for_ack: {
          std::cerr << "Requesting SASL PLAIN capability" << std::endl;
          auto &&get_if = std::get_if<std::string>(&packet.command);
          if (get_if && *get_if == "CAP" && packet.params.at(1) == "ACK" &&
              packet.trailing_param.has_value()) {
            if (*packet.trailing_param == "sasl") {
              sock.send(irc_data::packet{.command = "AUTHENTICATE",
                                         .params = {"PLAIN"}}
                            .serialize());
              state = irc_sasl::sasl_state::wait_for_authenticate;
            }
          }
          break;
        }
        case irc_sasl::sasl_state::wait_for_authenticate: {
          std::cerr << "Authenticating" << std::endl;
          auto &&get_if = std::get_if<std::string>(&packet.command);
          if (get_if && *get_if == "AUTHENTICATE" &&
              packet.params.at(0) == "+") {

            std::vector<char> base64_in{};
            U32 i = 0;

            for (char ch : account) {
              base64_in.emplace_back(ch);
              i++;
            }
            base64_in.emplace_back('\0');
            for (char ch : account) {
              base64_in.emplace_back(ch);
              i++;
            }
            base64_in.emplace_back('\0');
            for (char ch : password) {
              base64_in.emplace_back(ch);
              i++;
            }

            const auto base64_length = base64_in.size();
            constexpr auto CHUNKS_SIZE = 300;

            U8 base64_buf[CHUNKS_SIZE]{0};
            size_t encoded_final = 0;
            for (auto i = 0; i < base64_length; i += CHUNKS_SIZE) {
              auto offset_final = 0;

              for (auto offset = 0; offset < CHUNKS_SIZE; offset++) {
                if ((i + offset) < base64_length) {
                  // Final offset is set not to zero here
                  base64_buf[offset] = base64_in.at(i + offset);
                  offset_final = offset;
                } else {
                  // If offset is zero, final offset is zero here as well
                  base64_buf[offset] = 0;
                }
              }
              if (offset_final > 0) {
                constexpr auto FINAL_SIZE = (CHUNKS_SIZE / 3) * 4;
                U8 base64_out[FINAL_SIZE + 1]{0};
                EVP_EncodeBlock(base64_out, base64_buf, offset_final);

                sock.send(
                    irc_data::packet{.command = "AUTHENTICATE",
                                     .params = {std::string{
                                         reinterpret_cast<char *>(base64_out)}}}
                        .serialize());
              } else {
                sock.send(
                    irc_data::packet{.command = "AUTHENTICATE", .params = {"+"}}
                        .serialize());
              }
            }

            state = irc_sasl::sasl_state::wait_for_response;
          }
          break;
        }
        case irc_sasl::sasl_state::wait_for_response: {
          auto &&get_if = std::get_if<U32>(&packet.command);
          if (get_if) {
            if (*get_if == 903) {
              std::cerr << "Authentication successful!" << std::endl;
              sock.send(irc_data::packet{.command = "CAP", .params = {"END"}}
                            .serialize());
              state = irc_sasl::sasl_state::complete;
            }
          }
          break;
        }
        case irc_sasl::sasl_state::complete: {
          break;
        }
        }
      }

      auto t1 = std::chrono::steady_clock::now();
      if (t1 > (t0 + irc_sasl::max_sasl_time)) {
        throw std::runtime_error{"Authentication timed out or failed"};
      }
    }
  }
}