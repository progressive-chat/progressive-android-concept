#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encryption_verify_utils_validate(const std::string& input);
std::string encryption_verify_utils_process(const std::string& input);
json encryption_verify_utils_toJson(const std::string& input);

} // namespace progressive
