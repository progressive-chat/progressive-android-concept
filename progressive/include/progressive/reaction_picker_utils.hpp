#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool reaction_picker_utils_validate(const std::string& input);
std::string reaction_picker_utils_process(const std::string& input);
json reaction_picker_utils_toJson(const std::string& input);

} // namespace progressive
