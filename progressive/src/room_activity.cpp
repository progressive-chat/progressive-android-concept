#include "progressive/room_activity.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_activity_validate(const std::string& input) { return !input.empty(); }
std::string room_activity_process(const std::string& input) { return input; }
json room_activity_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
