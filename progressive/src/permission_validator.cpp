#include "progressive/permission_validator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool permission_validator_validate(const std::string& input) { return !input.empty(); }
std::string permission_validator_process(const std::string& input) { return input; }
json permission_validator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
