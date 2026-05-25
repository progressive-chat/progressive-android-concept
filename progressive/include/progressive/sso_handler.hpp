#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sso_handler_validate(const std::string& input);
std::string sso_handler_process(const std::string& input);
json sso_handler_toJson(const std::string& input);

} // namespace progressive
