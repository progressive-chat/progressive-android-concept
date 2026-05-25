#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool typing_sender_validate(const std::string& input);
std::string typing_sender_process(const std::string& input);
json typing_sender_toJson(const std::string& input);

} // namespace progressive
