#include "progressive/push_condition.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_condition_validate(const std::string& input) { return !input.empty(); }
std::string push_condition_process(const std::string& input) { return input; }
json push_condition_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
