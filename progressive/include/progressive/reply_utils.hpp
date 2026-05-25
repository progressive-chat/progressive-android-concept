#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool reply_utils_validate(const std::string& input);
std::string reply_utils_process(const std::string& input);
json reply_utils_toJson(const std::string& input);

} // namespace progressive
