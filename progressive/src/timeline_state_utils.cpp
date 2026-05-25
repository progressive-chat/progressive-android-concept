#include "progressive/timeline_state_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool timeline_state_utils_validate(const std::string& input) { return !input.empty(); }
std::string timeline_state_utils_process(const std::string& input) { return input; }
json timeline_state_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
