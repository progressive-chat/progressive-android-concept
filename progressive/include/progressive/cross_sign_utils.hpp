#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cross_sign_utils_validate(const std::string& input);
std::string cross_sign_utils_process(const std::string& input);
json cross_sign_utils_toJson(const std::string& input);

} // namespace progressive
