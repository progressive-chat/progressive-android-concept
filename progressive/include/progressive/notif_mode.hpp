#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notif_mode_validate(const std::string& input);
std::string notif_mode_process(const std::string& input);
json notif_mode_toJson(const std::string& input);

} // namespace progressive
