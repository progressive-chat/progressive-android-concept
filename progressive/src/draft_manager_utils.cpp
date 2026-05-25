#include "progressive/draft_manager_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool draft_manager_utils_validate(const std::string& input) { return !input.empty(); }
std::string draft_manager_utils_process(const std::string& input) { return input; }
json draft_manager_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
