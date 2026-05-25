#include "progressive/profile_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool profile_utils_validate(const std::string& i) { return !i.empty(); }
std::string profile_utils_process(const std::string& i) { return i; }
json profile_utils_toJson(const std::string& i) { return json::object(); }
}
