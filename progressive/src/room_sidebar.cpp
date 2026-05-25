#include "progressive/room_sidebar.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_sidebar_validate(const std::string& input) { return !input.empty(); }
std::string room_sidebar_process(const std::string& input) { return input; }
json room_sidebar_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
