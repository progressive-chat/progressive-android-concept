#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_color_gen_validate(const std::string& input);
std::string avatar_color_gen_process(const std::string& input);
json avatar_color_gen_toJson(const std::string& input);

} // namespace progressive
