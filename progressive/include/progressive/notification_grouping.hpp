#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_grouping_validate(const std::string& input);
std::string notification_grouping_process(const std::string& input);
json notification_grouping_toJson(const std::string& input);

} // namespace progressive
