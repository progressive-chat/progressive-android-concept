#include "progressive/room_member_sorter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_member_sorter_validate(const std::string& input) { return !input.empty(); }
std::string room_member_sorter_process(const std::string& input) { return input; }
json room_member_sorter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
