#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encryption_key_backup_validate(const std::string& input);
std::string encryption_key_backup_process(const std::string& input);
json encryption_key_backup_toJson(const std::string& input);

} // namespace progressive
