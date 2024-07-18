# Minimum version is CMake 3.26
cmake_minimum_required(VERSION 3.25)

# Export compile commands for the language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project instantiation
project(chlorobot VERSION 1.1.8.51)

# Fetch gRPC
option(USE_SYSTEM_GRPC "Use system installed gRPC" ON)
if(USE_SYSTEM_GRPC)
  # Find system-installed gRPC
    find_package(gRPC CONFIG REQUIRED)
else()
    include(FetchContent)
        FetchContent_Declare(
        gRPC
        GIT_REPOSITORY https://github.com/grpc/grpc
        GIT_TAG        v1.64.1
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(gRPC)
endif()
# Add Protobuf
find_package(Protobuf REQUIRED)
# Add OpenSSL
find_package(OpenSSL REQUIRED) 
# Add LibreTLS
pkg_check_modules(LibTLS REQUIRED libtls)

# Configure the project header
configure_file(include/configuration.txt
    ${PROJECT_SOURCE_DIR}/include/configuration.hpp)

# Build our main executable
add_executable(${PROJECT_NAME}
    src/tls_socket.cpp
    src/irc_data.cpp
    src/irc.cpp
    src/main.cpp
)

# Use C++20 on target too
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# Include headers here
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${LibTLS_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
)

protobuf_generate_cpp(TARGET ${PROJECT_NAME}
    IMPORT_DIRS ${PROJECT_SOURCE_DIR}/
    PROTOS ${PROJECT_SOURCE_DIR}/chlorobot_rpc.proto
    PROTOC_OUT_DIR ${PROJECT_SOURCE_DIR}/protos
)
# Finally link
target_link_libraries(${PROJECT_NAME}
    ${LibTLS_LIBRARIES}
    OpenSSL::SSL OpenSSL::Crypto
    gRPC::grpc++
    gRPC::grpc++_reflection
    ${Protobuf_LIBRARIES}
)
# Protocol buffer generation
protobuf_generate(TARGET ${PROJECT_NAME}
    IMPORT_DIRS ${PROJECT_SOURCE_DIR}/
    PROTOS ${PROJECT_SOURCE_DIR}/chlorobot_rpc.proto
    PROTOC_OUT_DIR ${PROJECT_SOURCE_DIR}/protos
)
protobuf_generate(
    TARGET ${PROJECT_NAME}
    IMPORT_DIRS ${PROJECT_SOURCE_DIR}/
    PROTOS ${PROJECT_SOURCE_DIR}/chlorobot_rpc.proto
    PROTOC_OUT_DIR ${PROJECT_SOURCE_DIR}/protos
    LANGUAGE grpc
    PLUGIN protoc-gen-grpc=$<TARGET_FILE:gRPC::grpc_cpp_plugin>
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
)
