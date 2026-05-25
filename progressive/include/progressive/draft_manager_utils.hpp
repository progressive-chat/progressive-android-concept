#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool draft_manager_utils_validate(const std::string& input);
std::string draft_manager_utils_process(const std::string& input);
json draft_manager_utils_toJson(const std::string& input);

} // namespace progressive
