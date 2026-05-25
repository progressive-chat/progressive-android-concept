#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool public_room_lister_validate(const std::string& input);
std::string public_room_lister_process(const std::string& input);
json public_room_lister_toJson(const std::string& input);

} // namespace progressive
