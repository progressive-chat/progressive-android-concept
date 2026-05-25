#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cache_invalidator_validate(const std::string& input);
std::string cache_invalidator_process(const std::string& input);
json cache_invalidator_toJson(const std::string& input);

} // namespace progressive
