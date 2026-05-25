#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool call_duration_validate(const std::string& input);
std::string call_duration_process(const std::string& input);
json call_duration_toJson(const std::string& input);

} // namespace progressive
