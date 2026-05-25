#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_picker_validate(const std::string& input);
std::string avatar_picker_process(const std::string& input);
json avatar_picker_toJson(const std::string& input);

} // namespace progressive
