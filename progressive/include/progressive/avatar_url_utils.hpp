#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_url_utils_validate(const std::string& input);
std::string avatar_url_utils_process(const std::string& input);
json avatar_url_utils_toJson(const std::string& input);

} // namespace progressive
