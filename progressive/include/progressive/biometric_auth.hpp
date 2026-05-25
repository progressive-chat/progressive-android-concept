#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool biometric_auth_validate(const std::string& input);
std::string biometric_auth_process(const std::string& input);
json biometric_auth_toJson(const std::string& input);

} // namespace progressive
