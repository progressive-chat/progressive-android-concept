#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_export_validate(const std::string& input);
std::string session_export_process(const std::string& input);
json session_export_toJson(const std::string& input);

} // namespace progressive
