#include "progressive/notification_badge.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_badge_validate(const std::string& input) { return !input.empty(); }
std::string notification_badge_process(const std::string& input) { return input; }
json notification_badge_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
