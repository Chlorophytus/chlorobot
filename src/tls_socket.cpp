#include "../include/tls_socket.hpp"
using namespace chlorobot;

void tls_socket::socket::_initialize_bio(const std::string &host,
                                         const std::string &port) {
  BIO_ADDRINFO *resources = nullptr;
  const BIO_ADDRINFO *ai;

  if (!BIO_lookup_ex(host.c_str(), port.c_str(), BIO_LOOKUP_CLIENT, AF_INET,
                     SOCK_STREAM, 0, &resources)) {
    throw std::runtime_error{"failed to look up IP address info of IRC server"};
  }

  for (ai = resources; ai != nullptr; ai = BIO_ADDRINFO_next(ai)) {
    _sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);

    SSLCHAR_ptr host_info{
        BIO_ADDR_hostname_string(BIO_ADDRINFO_address(ai), AF_INET),
        [](char *ptr) { OPENSSL_free(ptr); }};
    if (_sock < 0) {
      std::cerr << "'" << host_info.get()
                << "' socket setup failed, on to the next one" << std::endl;
      continue;
    }

    if (!BIO_connect(_sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY)) {
      std::cerr << "'" << host_info.get()
                << "' socket connect failed, on to the next one" << std::endl;
      BIO_closesocket(_sock);
      _sock = -1;
      continue;
    }

    if (!BIO_socket_nbio(_sock, 1)) {
      std::cerr << "'" << host_info.get()
                << "' socket set non-blocking failed, on to the next one"
                << std::endl;
      BIO_closesocket(_sock);
      _sock = -1;
      continue;
    }

    std::cerr << "'" << host_info.get() << "' socket initialization successful"
              << std::endl;
    break;
  }

  BIO_ADDRINFO_free(resources);

  if (_sock < 0) {
    throw std::runtime_error{
        "failed to set up SSL/TLS BIO's socket descriptor"};
  }

  _bio = BIO_new(BIO_s_socket());
  if (!_bio) {
    BIO_closesocket(_sock);
    _sock = -1;
    throw std::runtime_error{"failed to set up SSL/TLS BIO"};
  }
  BIO_set_fd(_bio, _sock, BIO_CLOSE);
}

tls_socket::poll_state tls_socket::socket::_handle_data(int resource) {
  fd_set fds;
  int width;
  int ssl_sock = SSL_get_fd(_ssl.get());
  width = ssl_sock + 1;
  FD_ZERO(&fds);
  FD_SET(ssl_sock, &fds);

  struct timeval tval{
      .tv_sec = 0,
      .tv_usec = tls_socket::io_timeout_microseconds,
  };

  switch (SSL_get_error(_ssl.get(), resource)) {
  case SSL_ERROR_WANT_WRITE: {
    select(width, 0, &fds, 0, &tval);
    return tls_socket::poll_state::retry;
  }
  case SSL_ERROR_WANT_READ: {
    select(width, &fds, 0, 0, &tval);
    return tls_socket::poll_state::retry;
  }
  case SSL_ERROR_NONE: {
    return tls_socket::poll_state::ok;
  }
  case SSL_ERROR_ZERO_RETURN: // EOF
    return tls_socket::poll_state::end_of_stream;
  default:
    return tls_socket::poll_state::error;
  }
}

tls_socket::socket::operator bool() {
  return _handle_data(0) != chlorobot::tls_socket::poll_state::end_of_stream;
}

void tls_socket::socket::connect(const std::string &host,
                                 const std::string &port) {
  if (_context) {
    std::cerr << "Socket already was connected" << std::endl;
    return;
  }

  _gracefully_disconnected = false;

  std::cerr << "Trying to connect socket..." << std::endl;
  _context.reset(SSL_CTX_new(TLS_client_method()));

  if (!_context) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to create TLS context"};
  }

  SSL_CTX_set_verify(_context.get(), SSL_VERIFY_PEER, 0);
  if (!SSL_CTX_set_default_verify_paths(_context.get())) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to set default trusted certificate store"};
  }

  if (!SSL_CTX_set_min_proto_version(_context.get(), TLS1_2_VERSION)) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to set minimum TLS protocol version"};
  }

  _ssl.reset(SSL_new(_context.get()));
  if (!_ssl) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to create SSL object"};
  }

  _initialize_bio(host, port);
  if (!_bio) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to create BIO object"};
  }
  SSL_set_bio(_ssl.get(), _bio, _bio);

  if (!SSL_set_tlsext_host_name(_ssl.get(), host.c_str())) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to set SNI hostname"};
  }

  if (!SSL_set1_host(_ssl.get(), host.c_str())) {
    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to set certificate verification hostname"};
  }

  int ret = 0;
  while ((ret = SSL_connect(_ssl.get())) != 1) {
    if (_handle_data(ret) == tls_socket::poll_state::retry) {
      continue;
    }

    ERR_print_errors_fp(stderr);
    throw std::runtime_error{"failed to connect"};
  }
}

void tls_socket::socket::send(const std::string &packet) {
  if (_handle_data(0) == tls_socket::poll_state::end_of_stream) {
    throw std::runtime_error{"Can't send after an EOF"};
  }
  char buffer[max_buffer_size]{0};
  size_t wrotebytes = 0;
  const size_t n = packet.size();

  packet.copy(buffer, n, 0);

#ifdef CHLOROBOT_TRACE
  std::cerr << "[SEND] " << buffer;
#endif

  while (!SSL_write_ex(_ssl.get(), buffer, n, &wrotebytes)) {
    const auto status = _handle_data(0);

    switch (status) {
    case tls_socket::poll_state::end_of_stream: {
      throw std::runtime_error{"Unexpected EOF encountered while writing"};
    }
    case tls_socket::poll_state::error: {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"TLS write failure"};
    }
    case tls_socket::poll_state::retry: {
      continue;
    }
    case tls_socket::poll_state::ok: {
      return;
    }
    }
  }
}
std::optional<std::string> tls_socket::socket::recv() {
  if (_handle_data(0) == tls_socket::poll_state::end_of_stream) {
    throw std::runtime_error{"Can't receive after an EOF"};
  }
  char buffer[max_buffer_size]{0};
  size_t readbytes = 0;

  while (!SSL_read_ex(_ssl.get(), buffer, sizeof(buffer), &readbytes)) {
    const auto status = _handle_data(0);

    switch (status) {
    case tls_socket::poll_state::end_of_stream: {
      throw std::runtime_error{"Unexpected EOF encountered while reading"};
    }
    case tls_socket::poll_state::error: {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"TLS read failure"};
    }
    case tls_socket::poll_state::retry: {
      continue;
    }
    case tls_socket::poll_state::ok: {
      break;
    }
    }
  }
  if (readbytes > 0) {
#ifdef CHLOROBOT_TRACE
    std::cerr << "[RECV] " << buffer;
#endif
    return std::string{buffer, readbytes};
  } else {
    return std::nullopt;
  }
}

void tls_socket::socket::disconnect() {
  if (!_context) {
    std::cerr << "Socket already was disconnected" << std::endl;
    return;
  }

  if (!_ssl) {
    throw std::runtime_error{"SSL does not exist but its context does"};
  }

  int ret_code;
  std::cerr << "Trying to shut down and disconnect socket gracefully"
            << std::endl;
  while ((ret_code = SSL_shutdown(_ssl.get()))) {
    const tls_socket::poll_state status = _handle_data(ret_code);

    if (ret_code < 0) {
      switch (status) {
      case tls_socket::poll_state::ok: {
        break;
      }
      case tls_socket::poll_state::retry: {
        continue;
      }
      case tls_socket::poll_state::end_of_stream: {
        break;
      }
      case tls_socket::poll_state::error: {
        break;
      }
      }

      if (ret_code < 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error{"Could not gracefully shut down"};
      }
    }
  }

  // BIO should be automatically freed. Please let that be true.
  _ssl = nullptr;
  _context = nullptr;
  _gracefully_disconnected = true;
}

bool tls_socket::socket::gracefully_disconnected() const {
  return _gracefully_disconnected;
}