#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_alias_validator_validate(const std::string& input);
std::string room_alias_validator_process(const std::string& input);
json room_alias_validator_toJson(const std::string& input);

} // namespace progressive
