#include "progressive/room_mute_scheduler.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_mute_scheduler_validate(const std::string& input) { return !input.empty(); }
std::string room_mute_scheduler_process(const std::string& input) { return input; }
json room_mute_scheduler_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
