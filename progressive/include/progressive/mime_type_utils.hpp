#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool mime_type_utils_validate(const std::string& input);
std::string mime_type_utils_process(const std::string& input);
json mime_type_utils_toJson(const std::string& input);

} // namespace progressive
