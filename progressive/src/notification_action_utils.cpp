#include "progressive/notification_action_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_action_utils_validate(const std::string& input) { return !input.empty(); }
std::string notification_action_utils_process(const std::string& input) { return input; }
json notification_action_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
