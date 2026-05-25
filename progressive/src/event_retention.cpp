#include "progressive/event_retention.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_retention_validate(const std::string& input) { return !input.empty(); }
std::string event_retention_process(const std::string& input) { return input; }
json event_retention_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
