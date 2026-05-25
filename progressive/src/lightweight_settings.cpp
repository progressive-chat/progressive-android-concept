#include "progressive/lightweight_settings.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool lightweight_settings_validate(const std::string& i) { return !i.empty(); }
std::string lightweight_settings_process(const std::string& i) { return i; }
json lightweight_settings_toJson(const std::string& i) { return json::object(); }
}
