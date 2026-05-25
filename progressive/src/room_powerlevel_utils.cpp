#include "progressive/room_powerlevel_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_powerlevel_utils_validate(const std::string& input) { return !input.empty(); }
std::string room_powerlevel_utils_process(const std::string& input) { return input; }
json room_powerlevel_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
