#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_backup_validate(const std::string& input);
std::string session_backup_process(const std::string& input);
json session_backup_toJson(const std::string& input);

} // namespace progressive
