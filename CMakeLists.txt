# Minimum version is CMake 3.26
cmake_minimum_required(VERSION 3.25)

# Export compile commands for the language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project instantiation
project(chlorobot VERSION 3.0.2.79)

# Add OpenSSL
find_package(OpenSSL REQUIRED) 
# Add LuaJIT
pkg_check_modules(LuaJIT REQUIRED luajit)

# Configure the project header
configure_file(include/configuration.txt
    ${PROJECT_SOURCE_DIR}/include/configuration.hpp)

# Build our main executable
add_executable(${PROJECT_NAME}
    src/scripting.cpp
    src/tls_socket.cpp
    src/irc_sasl.cpp
    src/irc_data.cpp
    src/irc.cpp
    src/main.cpp
)

# Use C++23 on target too
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

# Include headers here
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/include
    ${LuaJIT_INCLUDE_DIRS}
)
# Finally link
target_link_libraries(${PROJECT_NAME}
    ${LuaJIT_LIBRARIES}
    OpenSSL::SSL OpenSSL::Crypto
)
