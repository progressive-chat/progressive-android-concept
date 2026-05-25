#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool account_deactivation_utils_validate(const std::string& input);
std::string account_deactivation_utils_process(const std::string& input);
json account_deactivation_utils_toJson(const std::string& input);

} // namespace progressive
