#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_validate(const std::string& input);
std::string notification_process(const std::string& input);
json notification_toJson(const std::string& input);

} // namespace progressive
