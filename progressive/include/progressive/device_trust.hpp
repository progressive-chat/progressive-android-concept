#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_trust_validate(const std::string& input);
std::string device_trust_process(const std::string& input);
json device_trust_toJson(const std::string& input);

} // namespace progressive
