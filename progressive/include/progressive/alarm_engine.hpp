#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool alarm_engine_validate(const std::string& input);
std::string alarm_engine_process(const std::string& input);
json alarm_engine_toJson(const std::string& input);

} // namespace progressive
