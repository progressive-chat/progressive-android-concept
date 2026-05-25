#include "progressive/room_version.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool room_version_validate(const std::string& i) { return !i.empty(); }
std::string room_version_process(const std::string& i) { return i; }
json room_version_toJson(const std::string& i) { return json::object(); }
}
