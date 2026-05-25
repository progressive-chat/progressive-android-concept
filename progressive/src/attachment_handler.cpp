#include "progressive/attachment_handler.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool attachment_handler_validate(const std::string& i) { return !i.empty(); }
std::string attachment_handler_process(const std::string& i) { return i; }
json attachment_handler_toJson(const std::string& i) { return json::object(); }
}
