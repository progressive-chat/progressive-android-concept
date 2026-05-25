#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool presence_monitor_validate(const std::string& input);
std::string presence_monitor_process(const std::string& input);
json presence_monitor_toJson(const std::string& input);

} // namespace progressive
