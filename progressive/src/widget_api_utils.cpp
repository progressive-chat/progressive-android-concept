#include "progressive/widget_api_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_api_utils_validate(const std::string& input) { return !input.empty(); }
std::string widget_api_utils_process(const std::string& input) { return input; }
json widget_api_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
