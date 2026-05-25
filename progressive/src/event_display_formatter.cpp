#include "progressive/event_display_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_display_formatter_validate(const std::string& input) { return !input.empty(); }
std::string event_display_formatter_process(const std::string& input) { return input; }
json event_display_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
