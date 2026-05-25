#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool voice_message_utils_validate(const std::string& input);
std::string voice_message_utils_process(const std::string& input);
json voice_message_utils_toJson(const std::string& input);

} // namespace progressive
