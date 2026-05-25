#include "progressive/sync_scheduler.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sync_scheduler_validate(const std::string& input) { return !input.empty(); }
std::string sync_scheduler_process(const std::string& input) { return input; }
json sync_scheduler_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
