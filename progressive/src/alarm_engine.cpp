#include "progressive/alarm_engine.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool alarm_engine_validate(const std::string& input) { return !input.empty(); }
std::string alarm_engine_process(const std::string& input) { return input; }
json alarm_engine_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
