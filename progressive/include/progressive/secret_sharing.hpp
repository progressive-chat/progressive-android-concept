#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool secret_sharing_validate(const std::string& input);
std::string secret_sharing_process(const std::string& input);
json secret_sharing_toJson(const std::string& input);

} // namespace progressive
