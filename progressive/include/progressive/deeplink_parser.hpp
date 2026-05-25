#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool deeplink_parser_validate(const std::string& input);
std::string deeplink_parser_process(const std::string& input);
json deeplink_parser_toJson(const std::string& input);

} // namespace progressive
