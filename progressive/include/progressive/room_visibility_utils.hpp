#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_visibility_utils_validate(const std::string& input);
std::string room_visibility_utils_process(const std::string& input);
json room_visibility_utils_toJson(const std::string& input);

} // namespace progressive
