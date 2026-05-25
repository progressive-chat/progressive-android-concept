#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_utils_ext_validate(const std::string& input);
std::string media_utils_ext_process(const std::string& input);
json media_utils_ext_toJson(const std::string& input);

} // namespace progressive
