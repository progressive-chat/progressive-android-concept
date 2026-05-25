#include "progressive/account_validator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool account_validator_validate(const std::string& input) { return !input.empty(); }
std::string account_validator_process(const std::string& input) { return input; }
json account_validator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
