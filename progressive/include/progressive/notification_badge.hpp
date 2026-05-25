#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_badge_validate(const std::string& input);
std::string notification_badge_process(const std::string& input);
json notification_badge_toJson(const std::string& input);

} // namespace progressive
