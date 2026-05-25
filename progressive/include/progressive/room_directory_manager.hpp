#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_directory_manager_validate(const std::string& input);
std::string room_directory_manager_process(const std::string& input);
json room_directory_manager_toJson(const std::string& input);

} // namespace progressive
