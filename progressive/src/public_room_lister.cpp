#include "progressive/public_room_lister.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool public_room_lister_validate(const std::string& input) { return !input.empty(); }
std::string public_room_lister_process(const std::string& input) { return input; }
json public_room_lister_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
