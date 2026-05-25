#include "progressive/notification_filter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_filter_validate(const std::string& input) { return !input.empty(); }
std::string notification_filter_process(const std::string& input) { return input; }
json notification_filter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
