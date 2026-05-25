#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool dehydrate_utils_validate(const std::string& input);
std::string dehydrate_utils_process(const std::string& input);
json dehydrate_utils_toJson(const std::string& input);

} // namespace progressive
