#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool recovery_utils_validate(const std::string& input);
std::string recovery_utils_process(const std::string& input);
json recovery_utils_toJson(const std::string& input);

} // namespace progressive
