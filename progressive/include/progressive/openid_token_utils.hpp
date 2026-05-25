#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool openid_token_utils_validate(const std::string& input);
std::string openid_token_utils_process(const std::string& input);
json openid_token_utils_toJson(const std::string& input);

} // namespace progressive
