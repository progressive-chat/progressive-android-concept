#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool permission_validator_validate(const std::string& input);
std::string permission_validator_process(const std::string& input);
json permission_validator_toJson(const std::string& input);

} // namespace progressive
