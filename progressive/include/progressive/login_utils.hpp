#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool login_utils_validate(const std::string& input);
std::string login_utils_process(const std::string& input);
json login_utils_toJson(const std::string& input);

} // namespace progressive
