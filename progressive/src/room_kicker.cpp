#include "progressive/room_kicker.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_kicker_validate(const std::string& input) { return !input.empty(); }
std::string room_kicker_process(const std::string& input) { return input; }
json room_kicker_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
