#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool lightweight_settings_validate(const std::string& input);
std::string lightweight_settings_process(const std::string& input);
json lightweight_settings_toJson(const std::string& input);

} // namespace progressive
