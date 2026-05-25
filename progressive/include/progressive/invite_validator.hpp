#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool invite_validator_validate(const std::string& input);
std::string invite_validator_process(const std::string& input);
json invite_validator_toJson(const std::string& input);

} // namespace progressive
