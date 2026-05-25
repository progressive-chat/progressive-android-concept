#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool login_token_util_validate(const std::string& input);
std::string login_token_util_process(const std::string& input);
json login_token_util_toJson(const std::string& input);

} // namespace progressive
