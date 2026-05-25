#include "progressive/session_watchdog.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_watchdog_validate(const std::string& input) { return !input.empty(); }
std::string session_watchdog_process(const std::string& input) { return input; }
json session_watchdog_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
