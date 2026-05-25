#include "progressive/invite_validator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool invite_validator_validate(const std::string& input) { return !input.empty(); }
std::string invite_validator_process(const std::string& input) { return input; }
json invite_validator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
