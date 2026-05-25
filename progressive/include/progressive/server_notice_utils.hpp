#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_notice_utils_validate(const std::string& input);
std::string server_notice_utils_process(const std::string& input);
json server_notice_utils_toJson(const std::string& input);

} // namespace progressive
