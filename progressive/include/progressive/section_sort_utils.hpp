#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool section_sort_utils_validate(const std::string& input);
std::string section_sort_utils_process(const std::string& input);
json section_sort_utils_toJson(const std::string& input);

} // namespace progressive
