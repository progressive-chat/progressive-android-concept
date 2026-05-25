#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_previewer_validate(const std::string& input);
std::string room_previewer_process(const std::string& input);
json room_previewer_toJson(const std::string& input);

} // namespace progressive
