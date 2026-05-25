#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_reply_utils_validate(const std::string& input);
std::string message_reply_utils_process(const std::string& input);
json message_reply_utils_toJson(const std::string& input);

} // namespace progressive
