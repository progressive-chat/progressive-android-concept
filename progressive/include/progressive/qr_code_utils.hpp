#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool qr_code_utils_validate(const std::string& input);
std::string qr_code_utils_process(const std::string& input);
json qr_code_utils_toJson(const std::string& input);

} // namespace progressive
