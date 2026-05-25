#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_api_utils_validate(const std::string& input);
std::string widget_api_utils_process(const std::string& input);
json widget_api_utils_toJson(const std::string& input);

} // namespace progressive
