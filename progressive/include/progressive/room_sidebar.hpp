#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_sidebar_validate(const std::string& input);
std::string room_sidebar_process(const std::string& input);
json room_sidebar_toJson(const std::string& input);

} // namespace progressive
