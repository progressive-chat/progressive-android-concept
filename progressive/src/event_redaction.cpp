#include "progressive/event_redaction.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_redaction_validate(const std::string& input) { return !input.empty(); }
std::string event_redaction_process(const std::string& input) { return input; }
json event_redaction_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
