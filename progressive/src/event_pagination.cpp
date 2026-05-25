#include "progressive/event_pagination.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool event_pagination_validate(const std::string& i) { return !i.empty(); }
std::string event_pagination_process(const std::string& i) { return i; }
json event_pagination_toJson(const std::string& i) { return json::object(); }
}
