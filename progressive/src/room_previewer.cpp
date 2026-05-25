#include "progressive/room_previewer.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_previewer_validate(const std::string& input) { return !input.empty(); }
std::string room_previewer_process(const std::string& input) { return input; }
json room_previewer_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
