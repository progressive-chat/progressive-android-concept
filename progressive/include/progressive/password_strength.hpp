#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool password_strength_validate(const std::string& input);
std::string password_strength_process(const std::string& input);
json password_strength_toJson(const std::string& input);

} // namespace progressive
