#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_verifier_validate(const std::string& input);
std::string device_verifier_process(const std::string& input);
json device_verifier_toJson(const std::string& input);

} // namespace progressive
