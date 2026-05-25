#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_reminder_validate(const std::string& input);
std::string event_reminder_process(const std::string& input);
json event_reminder_toJson(const std::string& input);

} // namespace progressive
