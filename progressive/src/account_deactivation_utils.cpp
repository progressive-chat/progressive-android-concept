#include "progressive/account_deactivation_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool account_deactivation_utils_validate(const std::string& input) { return !input.empty(); }
std::string account_deactivation_utils_process(const std::string& input) { return input; }
json account_deactivation_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
