#include "progressive/key_export_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool key_export_utils_validate(const std::string& i) { return !i.empty(); }
std::string key_export_utils_process(const std::string& i) { return i; }
json key_export_utils_toJson(const std::string& i) { return json::object(); }
}
