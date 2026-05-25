#include "progressive/room_permission_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_permission_utils_validate(const std::string& input) { return !input.empty(); }
std::string room_permission_utils_process(const std::string& input) { return input; }
json room_permission_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
