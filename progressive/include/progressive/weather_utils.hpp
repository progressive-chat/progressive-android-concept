#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool weather_utils_validate(const std::string& input);
std::string weather_utils_process(const std::string& input);
json weather_utils_toJson(const std::string& input);

} // namespace progressive
