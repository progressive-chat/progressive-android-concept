#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool input_tools_validate(const std::string& input);
std::string input_tools_process(const std::string& input);
json input_tools_toJson(const std::string& input);

} // namespace progressive
