#include "progressive/event_context_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_context_utils_validate(const std::string& input) { return !input.empty(); }
std::string event_context_utils_process(const std::string& input) { return input; }
json event_context_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
