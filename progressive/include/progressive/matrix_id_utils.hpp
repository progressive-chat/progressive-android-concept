#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool matrix_id_utils_validate(const std::string& input);
std::string matrix_id_utils_process(const std::string& input);
json matrix_id_utils_toJson(const std::string& input);

} // namespace progressive
