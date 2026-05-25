#include "progressive/room_access_control.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_access_control_validate(const std::string& input) { return !input.empty(); }
std::string room_access_control_process(const std::string& input) { return input; }
json room_access_control_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
