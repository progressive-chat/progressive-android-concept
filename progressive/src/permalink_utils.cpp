#include "progressive/permalink_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool permalink_utils_validate(const std::string& i) { return !i.empty(); }
std::string permalink_utils_process(const std::string& i) { return i; }
json permalink_utils_toJson(const std::string& i) { return json::object(); }
}
