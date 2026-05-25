#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool secret_storage_utils_validate(const std::string& input);
std::string secret_storage_utils_process(const std::string& input);
json secret_storage_utils_toJson(const std::string& input);

} // namespace progressive
