#include "progressive/create_room.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool create_room_validate(const std::string& input) { return !input.empty(); }
std::string create_room_process(const std::string& input) { return input; }
json create_room_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
