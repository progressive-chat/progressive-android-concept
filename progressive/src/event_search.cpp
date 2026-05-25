#include "progressive/event_search.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_search_validate(const std::string& input) { return !input.empty(); }
std::string event_search_process(const std::string& input) { return input; }
json event_search_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
