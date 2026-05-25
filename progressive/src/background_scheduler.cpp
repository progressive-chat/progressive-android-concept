#include "progressive/background_scheduler.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool background_scheduler_validate(const std::string& input) { return !input.empty(); }
std::string background_scheduler_process(const std::string& input) { return input; }
json background_scheduler_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
