#include "progressive/room_autocomplete.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_autocomplete_validate(const std::string& input) { return !input.empty(); }
std::string room_autocomplete_process(const std::string& input) { return input; }
json room_autocomplete_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
