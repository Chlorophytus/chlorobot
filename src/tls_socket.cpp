#include "../include/tls_socket.hpp"
using namespace chlorobot;

using SSL_CTX_sptr = std::unique_ptr<SSL_CTX, std::function<void(SSL_CTX *)>>;
static SSL_CTX_sptr ctx;

using SSL_sptr = std::unique_ptr<SSL, std::function<void(SSL *)>>;
static SSL_sptr ssl;

using BIO_sptr = std::unique_ptr<BIO, std::function<void(BIO *)>>;
static BIO_sptr bio;

using SSLCHAR_sptr = std::unique_ptr<char, std::function<void(char *)>>;
constexpr static auto max_buffer_size = 4096;
constexpr static auto io_timeout_microseconds = 50'000;
static bool eof = false;

void initialize_bio(const std::string host, const std::string port) {
  BIO_ADDRINFO *resources = nullptr;
  const BIO_ADDRINFO *ai;
  bio = BIO_sptr{nullptr, [](BIO *p) {}};
  int sock = -1;

  if (!BIO_lookup_ex(host.c_str(), port.c_str(), BIO_LOOKUP_CLIENT, AF_INET,
                     SOCK_STREAM, 0, &resources)) {
    throw std::runtime_error{"failed to look up IP address info of IRC server"};
  }

  for (ai = resources; ai != nullptr; ai = BIO_ADDRINFO_next(ai)) {
    sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_STREAM, 0, 0);
    auto info = SSLCHAR_sptr{
        BIO_ADDR_hostname_string(BIO_ADDRINFO_address(ai), AF_INET),
        [](char *p) { OPENSSL_free(p); }};
    if (sock < 0) {
      std::cerr << "'" << info.get()
                << "' socket setup failed, on to the next one" << std::endl;
      continue;
    }

    if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), BIO_SOCK_NODELAY)) {
      std::cerr << "'" << info.get()
                << "' socket connect failed, on to the next one" << std::endl;
      BIO_closesocket(sock);
      sock = -1;
      continue;
    }

    if (!BIO_socket_nbio(sock, 1)) {
      std::cerr << "'" << info.get()
                << "' socket make non-blocking failed, on to the next one"
                << std::endl;
      sock = -1;
      continue;
    }

    std::cerr << "'" << info.get() << "' socket initialization successful"
              << std::endl;
    break;
  }

  BIO_ADDRINFO_free(resources);

  if (sock < 0) {
    throw std::runtime_error{"failed to set up IRC client BIO's socket"};
  }

  bio.reset(BIO_new(BIO_s_socket()));
  if (!bio) {
    BIO_closesocket(sock);
    throw std::runtime_error{"failed to set up IRC client BIO"};
  }
  BIO_set_fd(bio.get(), sock, BIO_CLOSE);
}

tls_socket::poll_state handle_data(int res) {
  fd_set fds;
  int width, sock;
  sock = SSL_get_fd(ssl.get());
  FD_ZERO(&fds);
  FD_SET(sock, &fds);
  width = sock + 1;

  struct timeval ts {
    .tv_sec = 0, .tv_usec = io_timeout_microseconds,
  };

  switch (SSL_get_error(ssl.get(), res)) {
  case SSL_ERROR_WANT_WRITE: {
    select(width, 0, &fds, 0, &ts);
    return tls_socket::poll_state::retry;
  }
  case SSL_ERROR_WANT_READ: {
    select(width, &fds, 0, 0, &ts);
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

void tls_socket::connect(const std::string host, const std::string port) {
  if (!ctx) {
    eof = false;
    std::cerr << "TLS: Create new context" << std::endl;
    ctx = SSL_CTX_sptr{SSL_CTX_new(TLS_client_method()),
                       [](SSL_CTX *p) { SSL_CTX_free(p); }};
    if (!ctx) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"failed to create TLS context"};
    }

    SSL_CTX_set_verify(ctx.get(), SSL_VERIFY_PEER, 0);
    // SSL_CTX_clear_mode(ctx.get(), SSL_MODE_AUTO_RETRY);

    if (!SSL_CTX_set_default_verify_paths(ctx.get())) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{
          "failed to set default trusted certificate store"};
    }

    if (!SSL_CTX_set_min_proto_version(ctx.get(), TLS1_2_VERSION)) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"failed to set minimum TLS protocol version"};
    }

    ssl = SSL_sptr{SSL_new(ctx.get()), [](SSL *p) { SSL_free(p); }};
    if (!ssl) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"failed to create SSL object"};
    }
    initialize_bio(host, port);
    if (!bio) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"failed to create BIO object"};
    }
    SSL_set_bio(ssl.get(), bio.get(), bio.get());

    if (!SSL_set_tlsext_host_name(ssl.get(), host.c_str())) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"failed to set SNI hostname"};
    }

    if (!SSL_set1_host(ssl.get(), host.c_str())) {
      ERR_print_errors_fp(stderr);
      throw std::runtime_error{
          "failed to set certificate verification hostname"};
    }
    int ret = 0;
    while ((ret = SSL_connect(ssl.get())) != 1) {
      if (handle_data(ret) == tls_socket::poll_state::retry) {
        continue;
      }

      ERR_print_errors_fp(stderr);
      throw std::runtime_error{"failed to connect"};
    }
  } else {
    throw std::runtime_error{"TLS singleton(s) already in use"};
  }
}
std::optional<std::string> tls_socket::recv() {
  if (is_eof()) {
    throw std::runtime_error{"Can't receive after EOF"};
  }
  char buffer[max_buffer_size]{0};
  size_t readbytes = 0;

  while (!SSL_read_ex(ssl.get(), buffer, sizeof(buffer), &readbytes)) {
    const auto status = handle_data(0);

    switch (status) {
    case tls_socket::poll_state::end_of_stream: {
      std::cerr << "EOF occured on read" << std::endl;
      eof = true;
      return std::nullopt;
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
    return std::string{buffer, readbytes};
  } else {
    return std::nullopt;
  }
}
void tls_socket::send(const std::string packet) {
  if (is_eof()) {
    throw std::runtime_error{"Can't send after EOF"};
  }
  char buffer[max_buffer_size]{0};
  size_t wrotebytes = 0;
  const size_t n = packet.size();

  packet.copy(buffer, n, 0);

  while (!SSL_write_ex(ssl.get(), buffer, n, &wrotebytes)) {
    const auto status = handle_data(0);

    switch (status) {
    case tls_socket::poll_state::end_of_stream: {
      std::cerr << "EOF occured on write" << std::endl;
      eof = true;
      return;
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
void tls_socket::disconnect() {
  if (ssl) {
    std::cerr << "Close SSL" << std::endl;
    int ret;
    while ((ret = SSL_shutdown(ssl.get())) != 1) {
      const auto status = handle_data(ret);
      if (ret < 0) {
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
      }

      if(ret < 0) {
        ERR_print_errors_fp(stderr);
        throw std::runtime_error{"Could not gracefully shutdown"};
      }
    }
    ssl = nullptr;
  }
  if (ctx) {
    std::cerr << "Free SSL context" << std::endl;
    ctx = nullptr;
  }
}
bool tls_socket::is_eof() { return eof; }
