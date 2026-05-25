#include "progressive/create_room.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool create_room_validate(const std::string& i) { return !i.empty(); }
std::string create_room_process(const std::string& i) { return i; }
json create_room_toJson(const std::string& i) { return json::object(); }
}
