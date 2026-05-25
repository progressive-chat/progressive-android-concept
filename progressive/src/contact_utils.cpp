#include "progressive/contact_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool contact_utils_validate(const std::string& i) { return !i.empty(); }
std::string contact_utils_process(const std::string& i) { return i; }
json contact_utils_toJson(const std::string& i) { return json::object(); }
}
