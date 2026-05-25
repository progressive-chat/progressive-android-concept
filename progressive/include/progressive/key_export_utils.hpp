#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool key_export_utils_validate(const std::string& input);
std::string key_export_utils_process(const std::string& input);
json key_export_utils_toJson(const std::string& input);

} // namespace progressive
