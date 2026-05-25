#include "progressive/device_naming_helper.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_naming_helper_validate(const std::string& input) { return !input.empty(); }
std::string device_naming_helper_process(const std::string& input) { return input; }
json device_naming_helper_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
