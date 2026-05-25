#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool profile_utils_validate(const std::string& input);
std::string profile_utils_process(const std::string& input);
json profile_utils_toJson(const std::string& input);

} // namespace progressive
