#include "progressive/server_version_check.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool server_version_check_validate(const std::string& i) { return !i.empty(); }
std::string server_version_check_process(const std::string& i) { return i; }
json server_version_check_toJson(const std::string& i) { return json::object(); }
}
