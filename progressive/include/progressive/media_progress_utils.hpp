#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_progress_utils_validate(const std::string& input);
std::string media_progress_utils_process(const std::string& input);
json media_progress_utils_toJson(const std::string& input);

} // namespace progressive
