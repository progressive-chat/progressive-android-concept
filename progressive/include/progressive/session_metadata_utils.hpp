#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_metadata_utils_validate(const std::string& input);
std::string session_metadata_utils_process(const std::string& input);
json session_metadata_utils_toJson(const std::string& input);

} // namespace progressive
