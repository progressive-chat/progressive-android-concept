#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool crypto_init_utils_validate(const std::string& input);
std::string crypto_init_utils_process(const std::string& input);
json crypto_init_utils_toJson(const std::string& input);

} // namespace progressive
