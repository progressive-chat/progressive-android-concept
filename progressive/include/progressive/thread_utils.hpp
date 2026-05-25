#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thread_utils_validate(const std::string& input);
std::string thread_utils_process(const std::string& input);
json thread_utils_toJson(const std::string& input);

} // namespace progressive
