#include "progressive/room_role_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool room_role_utils_validate(const std::string& i) { return !i.empty(); }
std::string room_role_utils_process(const std::string& i) { return i; }
json room_role_utils_toJson(const std::string& i) { return json::object(); }
}
