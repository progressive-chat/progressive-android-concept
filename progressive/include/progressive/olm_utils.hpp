#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool olm_utils_validate(const std::string& input);
std::string olm_utils_process(const std::string& input);
json olm_utils_toJson(const std::string& input);

} // namespace progressive
