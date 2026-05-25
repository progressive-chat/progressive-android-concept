#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_kicker_validate(const std::string& input);
std::string room_kicker_process(const std::string& input);
json room_kicker_toJson(const std::string& input);

} // namespace progressive
