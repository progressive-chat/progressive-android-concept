#include "progressive/room_power_level_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_power_level_utils_validate(const std::string& input) { return !input.empty(); }
std::string room_power_level_utils_process(const std::string& input) { return input; }
json room_power_level_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
