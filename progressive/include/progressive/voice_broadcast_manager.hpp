#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool voice_broadcast_manager_validate(const std::string& input);
std::string voice_broadcast_manager_process(const std::string& input);
json voice_broadcast_manager_toJson(const std::string& input);

} // namespace progressive
