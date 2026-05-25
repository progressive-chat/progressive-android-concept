#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_mute_scheduler_validate(const std::string& input);
std::string room_mute_scheduler_process(const std::string& input);
json room_mute_scheduler_toJson(const std::string& input);

} // namespace progressive
