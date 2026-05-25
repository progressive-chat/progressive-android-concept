#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_encryption_validate(const std::string& input);
std::string message_encryption_process(const std::string& input);
json message_encryption_toJson(const std::string& input);

} // namespace progressive
