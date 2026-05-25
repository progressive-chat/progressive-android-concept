#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool attachment_handler_validate(const std::string& input);
std::string attachment_handler_process(const std::string& input);
json attachment_handler_toJson(const std::string& input);

} // namespace progressive
