#include "progressive/message_edit_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool message_edit_utils_validate(const std::string& i) { return !i.empty(); }
std::string message_edit_utils_process(const std::string& i) { return i; }
json message_edit_utils_toJson(const std::string& i) { return json::object(); }
}
