#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool spotlight_search_utils_validate(const std::string& input);
std::string spotlight_search_utils_process(const std::string& input);
json spotlight_search_utils_toJson(const std::string& input);

} // namespace progressive
