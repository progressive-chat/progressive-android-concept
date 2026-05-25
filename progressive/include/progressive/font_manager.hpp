#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool font_manager_validate(const std::string& input);
std::string font_manager_process(const std::string& input);
json font_manager_toJson(const std::string& input);

} // namespace progressive
