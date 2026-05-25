#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool favorite_messages_validate(const std::string& input);
std::string favorite_messages_process(const std::string& input);
json favorite_messages_toJson(const std::string& input);

} // namespace progressive
