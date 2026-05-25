#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool backup_restore_utils_validate(const std::string& input);
std::string backup_restore_utils_process(const std::string& input);
json backup_restore_utils_toJson(const std::string& input);

} // namespace progressive
