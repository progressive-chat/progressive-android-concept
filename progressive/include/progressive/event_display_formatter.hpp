#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_display_formatter_validate(const std::string& input);
std::string event_display_formatter_process(const std::string& input);
json event_display_formatter_toJson(const std::string& input);

} // namespace progressive
