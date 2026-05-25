#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_summary_validate(const std::string& input);
std::string notification_summary_process(const std::string& input);
json notification_summary_toJson(const std::string& input);

} // namespace progressive
