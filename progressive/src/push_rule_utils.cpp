#include "progressive/push_rule_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_rule_utils_validate(const std::string& input) { return !input.empty(); }
std::string push_rule_utils_process(const std::string& input) { return input; }
json push_rule_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
