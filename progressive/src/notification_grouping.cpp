#include "progressive/notification_grouping.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_grouping_validate(const std::string& input) { return !input.empty(); }
std::string notification_grouping_process(const std::string& input) { return input; }
json notification_grouping_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
