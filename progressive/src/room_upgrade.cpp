#include "progressive/room_upgrade.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_upgrade_validate(const std::string& input) { return !input.empty(); }
std::string room_upgrade_process(const std::string& input) { return input; }
json room_upgrade_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
