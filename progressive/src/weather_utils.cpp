#include "progressive/weather_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool weather_utils_validate(const std::string& input) { return !input.empty(); }
std::string weather_utils_process(const std::string& input) { return input; }
json weather_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
