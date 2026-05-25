#include "progressive/room_read_marker_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_read_marker_utils_validate(const std::string& input) { return !input.empty(); }
std::string room_read_marker_utils_process(const std::string& input) { return input; }
json room_read_marker_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
