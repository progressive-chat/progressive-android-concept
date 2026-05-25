#include "progressive/ignore_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool ignore_utils_validate(const std::string& i) { return !i.empty(); }
std::string ignore_utils_process(const std::string& i) { return i; }
json ignore_utils_toJson(const std::string& i) { return json::object(); }
}
