#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_permission_utils_validate(const std::string& input);
std::string widget_permission_utils_process(const std::string& input);
json widget_permission_utils_toJson(const std::string& input);

} // namespace progressive
