#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_report_utils_validate(const std::string& input);
std::string message_report_utils_process(const std::string& input);
json message_report_utils_toJson(const std::string& input);

} // namespace progressive
