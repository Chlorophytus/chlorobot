#include "../include/tls_socket.hpp"
using namespace chlorobot;

static struct tls_config *config = nullptr;
static struct tls *client = nullptr;
static int write_fd, read_fd;
constexpr static auto max_buffer_size = 4096;

void irc::tls_socket::connect(const std::string host, const std::string port) {
  if (config == nullptr) {
    std::cerr << "TLS: Configure new object" << std::endl;
    config = tls_config_new();
    if (tls_config_error(config) != nullptr) {
      throw std::runtime_error{tls_config_error(config)};
    }

    std::cerr << "TLS: Set protocol" << std::endl;
    tls_config_set_protocols(config, TLS_PROTOCOL_TLSv1_3);
    if (tls_config_error(config) != nullptr) {
      throw std::runtime_error{tls_config_error(config)};
    }

    if (client == nullptr) {
      std::cerr << "TLS: Initialize client" << std::endl;
      client = tls_client();

      if (client == nullptr) {
        throw std::runtime_error{tls_error(client)};
      }

      std::cerr << "TLS: Configure client" << std::endl;
      auto error = tls_configure(client, config);
      if (error != 0) {
        throw std::runtime_error{tls_error(client)};
      }

      std::cerr << "TLS: Connect to " << host << ":" << port << std::endl;
      error = tls_connect(client, host.c_str(), port.c_str());
      if (error != 0) {
        throw std::runtime_error{tls_error(client)};
      }

      std::cerr << "TLS: Successfully connected!" << std::endl;
    } else {
      throw std::runtime_error{"TLS client singleton already in use"};
    }
  } else {
    throw std::runtime_error{"TLS configuration singleton already in use"};
  }
}
std::optional<std::string> irc::tls_socket::recv() {
  char buffer[max_buffer_size]{0};
  const auto n = tls_read(client, buffer, max_buffer_size - 1);
  if(n > 0) {
    return std::string{buffer, static_cast<U64>(n)};
  } else {
    return std::nullopt;
  }
}

void irc::tls_socket::send(const std::string message) {
  char buffer[max_buffer_size]{0};
  const auto n = message.size();
  if (n >= max_buffer_size) {
    throw std::runtime_error{"Send buffer overrun"};
  }
  message.copy(buffer, n, 0);
  tls_write(client, buffer, n);
}

void irc::tls_socket::disconnect() {
  if (client != nullptr) {
    std::cerr << "TLS: Close client" << std::endl;
    tls_close(client);
    client = nullptr;
  }
  if (config != nullptr) {
    std::cerr << "TLS: Free config" << std::endl;
    tls_config_free(config);
    config = nullptr;
  }
}