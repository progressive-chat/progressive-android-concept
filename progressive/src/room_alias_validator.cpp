#include "progressive/room_alias_validator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_alias_validator_validate(const std::string& input) { return !input.empty(); }
std::string room_alias_validator_process(const std::string& input) { return input; }
json room_alias_validator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
