#include "progressive/room_child_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool room_child_utils_validate(const std::string& i) { return !i.empty(); }
std::string room_child_utils_process(const std::string& i) { return i; }
json room_child_utils_toJson(const std::string& i) { return json::object(); }
}
