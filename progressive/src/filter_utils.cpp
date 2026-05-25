#include "progressive/filter_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool filter_utils_validate(const std::string& i) { return !i.empty(); }
std::string filter_utils_process(const std::string& i) { return i; }
json filter_utils_toJson(const std::string& i) { return json::object(); }
}
