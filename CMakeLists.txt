# Minimum version is CMake 3.26
cmake_minimum_required(VERSION 3.26)

# Export compile commands for the language server
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project instantiation
project(chlorobot VERSION 0.1.0.3)

# Add Lua
find_package(Lua 5.4 REQUIRED)
# Add Boost
find_package(Boost 1.83 REQUIRED)
find_package(OpenSSL REQUIRED)

# Not going to use this unless SASLprep is needed
#find_package(ICU REQUIRED COMPONENTS uc dt in io)

# Configure the project header
configure_file(include/configuration.txt
    ${PROJECT_SOURCE_DIR}/include/configuration.hpp)

# Build our main executable
add_executable(${PROJECT_NAME}
    src/irc.cpp
    src/main.cpp)

# Use C++20 on target too
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED TRUE)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)

# Include headers here
target_include_directories(${PROJECT_NAME} PRIVATE include
    ${LUA_INCLUDE_DIR} 
    ${Boost_INCLUDE_DIRS}
#    ${ICU_INCLUDE_DIRS}
)

# Finally link
target_link_libraries(${PROJECT_NAME}
    ${LUA_LIBRARIES}
    ${Boost_LIBRARIES}
#    ${ICU_LIBRARIES}
    OpenSSL::SSL
)