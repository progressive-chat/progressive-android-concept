#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool account_data_utils_validate(const std::string& input);
std::string account_data_utils_process(const std::string& input);
json account_data_utils_toJson(const std::string& input);

} // namespace progressive
