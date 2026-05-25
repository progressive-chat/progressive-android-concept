#include "progressive/widget_permission_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_permission_utils_validate(const std::string& input) { return !input.empty(); }
std::string widget_permission_utils_process(const std::string& input) { return input; }
json widget_permission_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
