#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_activity_validate(const std::string& input);
std::string room_activity_process(const std::string& input);
json room_activity_toJson(const std::string& input);

} // namespace progressive
