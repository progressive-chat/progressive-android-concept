#include "progressive/mime_type_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool mime_type_utils_validate(const std::string& i) { return !i.empty(); }
std::string mime_type_utils_process(const std::string& i) { return i; }
json mime_type_utils_toJson(const std::string& i) { return json::object(); }
}
