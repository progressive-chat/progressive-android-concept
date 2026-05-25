#include "progressive/room_child_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_child_utils_validate(const std::string& input) { return !input.empty(); }
std::string room_child_utils_process(const std::string& input) { return input; }
json room_child_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
