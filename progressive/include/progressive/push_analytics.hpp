#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_analytics_validate(const std::string& input);
std::string push_analytics_process(const std::string& input);
json push_analytics_toJson(const std::string& input);

} // namespace progressive
