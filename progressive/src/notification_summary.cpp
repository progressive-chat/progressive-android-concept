#include "progressive/notification_summary.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool notification_summary_validate(const std::string& i) { return !i.empty(); }
std::string notification_summary_process(const std::string& i) { return i; }
json notification_summary_toJson(const std::string& i) { return json::object(); }
}
