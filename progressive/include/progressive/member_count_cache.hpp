#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool member_count_cache_validate(const std::string& input);
std::string member_count_cache_process(const std::string& input);
json member_count_cache_toJson(const std::string& input);

} // namespace progressive
