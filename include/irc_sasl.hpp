#pragma once
#include "main.hpp"
namespace chlorobot {
namespace irc_sasl {
constexpr auto max_capability_time = std::chrono::seconds(20);
constexpr auto max_sasl_time = std::chrono::seconds(5);

enum class sasl_state {
    wait_for_ack,
    wait_for_authenticate,
    wait_for_response,
    complete
};

/// @brief Sends and checks for the SASL PLAIN capability
/// @return whether or not SASL PLAIN is supported
bool has_sasl_capability();
/// @brief Tries to acknowledge capabilities and a SASL header to the server
/// @param account SASL username
/// @param password SASL password
void try_auth(const std::string &account, const std::string &password);
} // namespace irc_sasl
} // namespace chlorobot
