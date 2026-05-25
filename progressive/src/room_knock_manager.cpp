#include "progressive/room_knock_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_knock_manager_validate(const std::string& input) { return !input.empty(); }
std::string room_knock_manager_process(const std::string& input) { return input; }
json room_knock_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
