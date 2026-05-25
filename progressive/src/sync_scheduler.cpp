#include "progressive/sync_scheduler.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool sync_scheduler_validate(const std::string& i) { return !i.empty(); }
std::string sync_scheduler_process(const std::string& i) { return i; }
json sync_scheduler_toJson(const std::string& i) { return json::object(); }
}
