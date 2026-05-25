#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_manager_ext_validate(const std::string& input);
std::string widget_manager_ext_process(const std::string& input);
json widget_manager_ext_toJson(const std::string& input);

} // namespace progressive
