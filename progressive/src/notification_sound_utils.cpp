#include "progressive/notification_sound_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool notification_sound_utils_validate(const std::string& i) { return !i.empty(); }
std::string notification_sound_utils_process(const std::string& i) { return i; }
json notification_sound_utils_toJson(const std::string& i) { return json::object(); }
}
