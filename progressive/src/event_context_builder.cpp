#include "progressive/event_context_builder.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_context_builder_validate(const std::string& input) { return !input.empty(); }
std::string event_context_builder_process(const std::string& input) { return input; }
json event_context_builder_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
