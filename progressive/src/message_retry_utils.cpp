#include "progressive/message_retry_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool message_retry_utils_validate(const std::string& i) { return !i.empty(); }
std::string message_retry_utils_process(const std::string& i) { return i; }
json message_retry_utils_toJson(const std::string& i) { return json::object(); }
}
