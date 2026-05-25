#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool location_share_validate(const std::string& input);
std::string location_share_process(const std::string& input);
json location_share_toJson(const std::string& input);

} // namespace progressive
