#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool matrix_error_validate(const std::string& input);
std::string matrix_error_process(const std::string& input);
json matrix_error_toJson(const std::string& input);

} // namespace progressive
