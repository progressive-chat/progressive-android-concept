#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool call_event_formatter_validate(const std::string& input);
std::string call_event_formatter_process(const std::string& input);
json call_event_formatter_toJson(const std::string& input);

} // namespace progressive
