#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_signout_validate(const std::string& input);
std::string session_signout_process(const std::string& input);
json session_signout_toJson(const std::string& input);

} // namespace progressive
