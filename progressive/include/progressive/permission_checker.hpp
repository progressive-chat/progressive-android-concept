#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool permission_checker_validate(const std::string& input);
std::string permission_checker_process(const std::string& input);
json permission_checker_toJson(const std::string& input);

} // namespace progressive
