#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sso_provider_utils_validate(const std::string& input);
std::string sso_provider_utils_process(const std::string& input);
json sso_provider_utils_toJson(const std::string& input);

} // namespace progressive
