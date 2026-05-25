#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool display_name_utils_validate(const std::string& input);
std::string display_name_utils_process(const std::string& input);
json display_name_utils_toJson(const std::string& input);

} // namespace progressive
