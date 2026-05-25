#include "progressive/message_search.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool message_search_validate(const std::string& i) { return !i.empty(); }
std::string message_search_process(const std::string& i) { return i; }
json message_search_toJson(const std::string& i) { return json::object(); }
}
