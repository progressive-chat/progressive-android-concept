#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_upgrade_validate(const std::string& input);
std::string room_upgrade_process(const std::string& input);
json room_upgrade_toJson(const std::string& input);

} // namespace progressive
