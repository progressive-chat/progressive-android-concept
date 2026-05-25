#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_cropper_validate(const std::string& input);
std::string avatar_cropper_process(const std::string& input);
json avatar_cropper_toJson(const std::string& input);

} // namespace progressive
