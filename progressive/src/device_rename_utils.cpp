#include "progressive/device_rename_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_rename_utils_validate(const std::string& input) { return !input.empty(); }
std::string device_rename_utils_process(const std::string& input) { return input; }
json device_rename_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
