#include "../include/tls_socket.hpp"
#include <fcntl.h>
#include <netdb.h>
#include <sys/poll.h>
using namespace chlorobot;

struct tls_config *config = nullptr;
struct tls *client = nullptr;
int client_fd;
constexpr auto max_buffer_size = 4096;
constexpr auto io_timeout_milliseconds = 50;

void tls_socket::connect(const std::string host, const std::string port) {
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

      std::cerr << "TLS: Resolve " << host << std::endl;
      struct addrinfo hints;
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = 0;
      hints.ai_protocol = 0;
      struct addrinfo *result;

      error = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
      if (error != 0) {
        throw std::runtime_error{gai_strerror(error)};
      }

      std::cerr << "TLS: Make TCP socket file descriptor" << std::endl;
      client_fd = socket(PF_INET, SOCK_STREAM, 0);
      if(client_fd == -1) {
        throw std::runtime_error{strerror(errno)};
      }

      std::cerr << "TLS: Connect TCP socket" << std::endl;
      error = connect(client_fd, result->ai_addr, result->ai_addrlen);

      if (error == -1) {
        throw std::runtime_error{strerror(errno)};
      }

      std::cerr << "TLS: Upgrading..." << std::endl;
      error = tls_connect_socket(client, client_fd, host.c_str());
      if (error != 0) {
        throw std::runtime_error{tls_error(client)};
      }

      std::cerr << "TLS: Successfully connected, making non-blocking..."
                << std::endl;
      int flags = fcntl(client_fd, F_GETFL, 0);
      if (flags < 0) {
        throw std::runtime_error{"Can't get socket file descriptor flags"};
      }
      flags |= O_NONBLOCK;
      flags = fcntl(client_fd, F_SETFL, flags);
      if (flags < 0) {
        throw std::runtime_error{"Can't set socket file descriptor flags"};
      }
      std::cerr << "TLS: Now non-blocking!" << std::endl;

      freeaddrinfo(result);
    } else {
      throw std::runtime_error{"TLS client singleton already in use"};
    }
  } else {
    throw std::runtime_error{"TLS configuration singleton already in use"};
  }
}
std::optional<std::string> tls_socket::recv() {
  struct pollfd pfd[1];
  pfd[0].fd = client_fd;
  pfd[0].events = POLLIN | POLLOUT;
  pfd[0].revents = 0;

  auto poller = 0;
  char buffer[max_buffer_size]{0};

  do {
    poller = poll(pfd, 1, io_timeout_milliseconds);
    if (poller == -1) {
      throw std::runtime_error{"Receiver poll"};
    } else if (pfd[0].revents & (POLLERR | POLLNVAL)) {
      throw std::runtime_error{"Receiver poll file descriptor"};
    } else if (pfd[0].revents & (POLLIN | POLLOUT | POLLHUP)) {
      auto reader = tls_read(client, buffer, max_buffer_size - 1);

      if (reader == TLS_WANT_POLLIN) {
        pfd[0].events = POLLIN;
      } else if (reader == TLS_WANT_POLLOUT) {
        pfd[0].events = POLLOUT;
      } else if (reader == -1) {
        throw std::runtime_error{tls_error(client)};
      } else {
        if (reader > 0) {
          return std::string{buffer, static_cast<U64>(reader)};
        }
      }
    }
  } while (poller > 0);

  return std::nullopt;
}

void tls_socket::send(const std::string message) {
  struct pollfd pfd[1];
  pfd[0].fd = client_fd;
  pfd[0].events = POLLIN | POLLOUT;
  pfd[0].revents = 0;

  auto poller = 0;
  char buffer[max_buffer_size]{0};
  const auto n = message.size();
  if (n >= max_buffer_size) {
    throw std::runtime_error{"Transmit buffer overrun"};
  }
  message.copy(buffer, n, 0);

  do {
    poller = poll(pfd, 1, io_timeout_milliseconds);
    if (poller == -1) {
      throw std::runtime_error{"Transmitter poll"};
    } else if (pfd[0].revents & (POLLERR | POLLNVAL)) {
      throw std::runtime_error{"Transmitter poll file descriptor"};
    } else if (pfd[0].revents & (POLLIN | POLLOUT | POLLHUP)) {
      auto writer = tls_write(client, buffer, n);

      if (writer == TLS_WANT_POLLIN) {
        pfd[0].events = POLLIN;
      } else if (writer == TLS_WANT_POLLOUT) {
        pfd[0].events = POLLOUT;
      } else if (writer == -1) {
        throw std::runtime_error{tls_error(client)};
      } else {
        return;
      }
    }
  } while (poller > 0);
}

void tls_socket::disconnect() {
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