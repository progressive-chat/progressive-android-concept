#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool translation_utils_validate(const std::string& input);
std::string translation_utils_process(const std::string& input);
json translation_utils_toJson(const std::string& input);

} // namespace progressive
