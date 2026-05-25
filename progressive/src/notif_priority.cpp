#include "progressive/notif_priority.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool notif_priority_validate(const std::string& i) { return !i.empty(); }
std::string notif_priority_process(const std::string& i) { return i; }
json notif_priority_toJson(const std::string& i) { return json::object(); }
}
