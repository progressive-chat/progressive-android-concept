#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_edit_utils_validate(const std::string& input);
std::string message_edit_utils_process(const std::string& input);
json message_edit_utils_toJson(const std::string& input);

} // namespace progressive
