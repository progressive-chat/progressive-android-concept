#include "progressive/notification_summary.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_summary_validate(const std::string& input) { return !input.empty(); }
std::string notification_summary_process(const std::string& input) { return input; }
json notification_summary_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
