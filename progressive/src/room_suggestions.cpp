#include "progressive/room_suggestions.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_suggestions_validate(const std::string& input) { return !input.empty(); }
std::string room_suggestions_process(const std::string& input) { return input; }
json room_suggestions_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
