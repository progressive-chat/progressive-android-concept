#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_version_check_validate(const std::string& input);
std::string server_version_check_process(const std::string& input);
json server_version_check_toJson(const std::string& input);

} // namespace progressive
