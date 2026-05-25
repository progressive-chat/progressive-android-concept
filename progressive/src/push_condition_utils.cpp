#include "progressive/push_condition_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_condition_utils_validate(const std::string& input) { return !input.empty(); }
std::string push_condition_utils_process(const std::string& input) { return input; }
json push_condition_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
