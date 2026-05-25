#include "progressive/secrets_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool secrets_utils_validate(const std::string& i) { return !i.empty(); }
std::string secrets_utils_process(const std::string& i) { return i; }
json secrets_utils_toJson(const std::string& i) { return json::object(); }
}
