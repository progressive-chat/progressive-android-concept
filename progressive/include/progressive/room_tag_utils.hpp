#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_tag_utils_validate(const std::string& input);
std::string room_tag_utils_process(const std::string& input);
json room_tag_utils_toJson(const std::string& input);

} // namespace progressive
