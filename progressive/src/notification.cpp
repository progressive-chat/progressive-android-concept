#include "progressive/notification.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_validate(const std::string& input) { return !input.empty(); }
std::string notification_process(const std::string& input) { return input; }
json notification_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
