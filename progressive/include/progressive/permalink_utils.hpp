#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool permalink_utils_validate(const std::string& input);
std::string permalink_utils_process(const std::string& input);
json permalink_utils_toJson(const std::string& input);

} // namespace progressive
