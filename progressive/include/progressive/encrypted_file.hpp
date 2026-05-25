#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encrypted_file_validate(const std::string& input);
std::string encrypted_file_process(const std::string& input);
json encrypted_file_toJson(const std::string& input);

} // namespace progressive
