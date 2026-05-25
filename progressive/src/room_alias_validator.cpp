#include "progressive/room_alias_validator.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool room_alias_validator_validate(const std::string& i) { return !i.empty(); }
std::string room_alias_validator_process(const std::string& i) { return i; }
json room_alias_validator_toJson(const std::string& i) { return json::object(); }
}
