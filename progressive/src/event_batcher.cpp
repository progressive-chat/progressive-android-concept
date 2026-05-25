#include "progressive/event_batcher.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_batcher_validate(const std::string& input) { return !input.empty(); }
std::string event_batcher_process(const std::string& input) { return input; }
json event_batcher_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
