#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool markdown_utils_validate(const std::string& input);
std::string markdown_utils_process(const std::string& input);
json markdown_utils_toJson(const std::string& input);

} // namespace progressive
