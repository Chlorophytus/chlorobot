#pragma once
// Auto-generated configuration is included
#include "configuration.hpp"

#include <chrono>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>

// OpenSSL SSL framework is included
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

// OpenSSL Base64 is included
#include <openssl/evp.h>

// Use sockets
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>

// Use LuaJIT
#include <lua.hpp>

// Unsigned 8-bit integer scalar
using U8 = std::uint8_t;
// Unsigned 16-bit integer scalar
using U16 = std::uint16_t;
// Unsigned 32-bit integer scalar
using U32 = std::uint32_t;
// Unsigned 64-bit integer scalar
using U64 = std::uint64_t;

// Signed 8-bit integer scalar
using S8 = std::int8_t;
// Signed 16-bit integer scalar
using S16 = std::int16_t;
// Signed 32-bit integer scalar
using S32 = std::int32_t;
// Signed 64-bit integer scalar
using S64 = std::int64_t;

// Floating-point 32-bit scalar
using F32 = float;
// Floating-point 64-bit scalar
using F64 = double;
