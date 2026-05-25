#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool read_marker_utils_validate(const std::string& input);
std::string read_marker_utils_process(const std::string& input);
json read_marker_utils_toJson(const std::string& input);

} // namespace progressive
