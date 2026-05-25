#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool lang_detect_validate(const std::string& input);
std::string lang_detect_process(const std::string& input);
json lang_detect_toJson(const std::string& input);

} // namespace progressive
