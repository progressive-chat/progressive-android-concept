#include "progressive/call_event_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool call_event_formatter_validate(const std::string& input) { return !input.empty(); }
std::string call_event_formatter_process(const std::string& input) { return input; }
json call_event_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
