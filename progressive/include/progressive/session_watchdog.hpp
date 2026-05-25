#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_watchdog_validate(const std::string& input);
std::string session_watchdog_process(const std::string& input);
json session_watchdog_toJson(const std::string& input);

} // namespace progressive
