#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool oidc_manager_validate(const std::string& input);
std::string oidc_manager_process(const std::string& input);
json oidc_manager_toJson(const std::string& input);

} // namespace progressive
