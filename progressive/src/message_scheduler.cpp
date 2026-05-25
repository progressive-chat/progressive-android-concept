#include "progressive/message_scheduler.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_scheduler_validate(const std::string& input) { return !input.empty(); }
std::string message_scheduler_process(const std::string& input) { return input; }
json message_scheduler_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
