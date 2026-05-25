#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_version_utils_validate(const std::string& input);
std::string event_version_utils_process(const std::string& input);
json event_version_utils_toJson(const std::string& input);

} // namespace progressive
