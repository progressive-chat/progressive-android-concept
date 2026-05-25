#include "progressive/event_age_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_age_utils_validate(const std::string& input) { return !input.empty(); }
std::string event_age_utils_process(const std::string& input) { return input; }
json event_age_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
