#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_restore_validate(const std::string& input);
std::string session_restore_process(const std::string& input);
json session_restore_toJson(const std::string& input);

} // namespace progressive
