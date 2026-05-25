#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool key_share_validate(const std::string& input);
std::string key_share_process(const std::string& input);
json key_share_toJson(const std::string& input);

} // namespace progressive
