#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool link_preview_utils_validate(const std::string& input);
std::string link_preview_utils_process(const std::string& input);
json link_preview_utils_toJson(const std::string& input);

} // namespace progressive
