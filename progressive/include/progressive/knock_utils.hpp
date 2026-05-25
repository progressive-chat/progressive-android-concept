#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool knock_utils_validate(const std::string& input);
std::string knock_utils_process(const std::string& input);
json knock_utils_toJson(const std::string& input);

} // namespace progressive
