#include "progressive/call_duration.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool call_duration_validate(const std::string& input) { return !input.empty(); }
std::string call_duration_process(const std::string& input) { return input; }
json call_duration_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
