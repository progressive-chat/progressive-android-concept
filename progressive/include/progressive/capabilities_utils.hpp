#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool capabilities_utils_validate(const std::string& input);
std::string capabilities_utils_process(const std::string& input);
json capabilities_utils_toJson(const std::string& input);

} // namespace progressive
