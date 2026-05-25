#include "progressive/presence_monitor.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool presence_monitor_validate(const std::string& input) { return !input.empty(); }
std::string presence_monitor_process(const std::string& input) { return input; }
json presence_monitor_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
