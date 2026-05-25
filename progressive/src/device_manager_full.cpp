#include "progressive/device_manager_full.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_manager_full_validate(const std::string& input) { return !input.empty(); }
std::string device_manager_full_process(const std::string& input) { return input; }
json device_manager_full_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
