#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_member_sorter_validate(const std::string& input);
std::string room_member_sorter_process(const std::string& input);
json room_member_sorter_toJson(const std::string& input);

} // namespace progressive
