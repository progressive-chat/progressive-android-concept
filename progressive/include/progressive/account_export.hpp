#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool account_export_validate(const std::string& input);
std::string account_export_process(const std::string& input);
json account_export_toJson(const std::string& input);

} // namespace progressive
