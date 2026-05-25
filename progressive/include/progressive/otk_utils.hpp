#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool otk_utils_validate(const std::string& input);
std::string otk_utils_process(const std::string& input);
json otk_utils_toJson(const std::string& input);

} // namespace progressive
