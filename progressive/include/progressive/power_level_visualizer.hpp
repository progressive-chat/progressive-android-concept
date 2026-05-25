#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool power_level_visualizer_validate(const std::string& input);
std::string power_level_visualizer_process(const std::string& input);
json power_level_visualizer_toJson(const std::string& input);

} // namespace progressive
