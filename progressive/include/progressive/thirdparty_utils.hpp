#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thirdparty_utils_validate(const std::string& input);
std::string thirdparty_utils_process(const std::string& input);
json thirdparty_utils_toJson(const std::string& input);

} // namespace progressive
