#include "progressive/push_analytics.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_analytics_validate(const std::string& input) { return !input.empty(); }
std::string push_analytics_process(const std::string& input) { return input; }
json push_analytics_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
