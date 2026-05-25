#include "progressive/font_manager.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool font_manager_validate(const std::string& i) { return !i.empty(); }
std::string font_manager_process(const std::string& i) { return i; }
json font_manager_toJson(const std::string& i) { return json::object(); }
}
