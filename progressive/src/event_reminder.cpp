#include "progressive/event_reminder.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_reminder_validate(const std::string& input) { return !input.empty(); }
std::string event_reminder_process(const std::string& input) { return input; }
json event_reminder_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
