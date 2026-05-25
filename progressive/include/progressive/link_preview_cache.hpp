#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool link_preview_cache_validate(const std::string& input);
std::string link_preview_cache_process(const std::string& input);
json link_preview_cache_toJson(const std::string& input);

} // namespace progressive
