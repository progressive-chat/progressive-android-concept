#include "progressive/event_pagination.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_pagination_validate(const std::string& input) { return !input.empty(); }
std::string event_pagination_process(const std::string& input) { return input; }
json event_pagination_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
