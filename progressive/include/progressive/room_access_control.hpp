#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_access_control_validate(const std::string& input);
std::string room_access_control_process(const std::string& input);
json room_access_control_toJson(const std::string& input);

} // namespace progressive
