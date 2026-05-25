#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sync_filter_utils_validate(const std::string& input);
std::string sync_filter_utils_process(const std::string& input);
json sync_filter_utils_toJson(const std::string& input);

} // namespace progressive
