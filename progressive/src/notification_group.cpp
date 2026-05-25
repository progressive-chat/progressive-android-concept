#include "progressive/notification_group.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_group_validate(const std::string& input) { return !input.empty(); }
std::string notification_group_process(const std::string& input) { return input; }
json notification_group_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
