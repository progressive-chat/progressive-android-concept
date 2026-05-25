#include "progressive/event_filter_utils.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool event_filter_utils_validate(const std::string& i) { return !i.empty(); }
std::string event_filter_utils_process(const std::string& i) { return i; }
json event_filter_utils_toJson(const std::string& i) { return json::object(); }
}
