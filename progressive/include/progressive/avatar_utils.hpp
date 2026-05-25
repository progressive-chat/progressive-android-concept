#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_utils_validate(const std::string& input);
std::string avatar_utils_process(const std::string& input);
json avatar_utils_toJson(const std::string& input);

} // namespace progressive
