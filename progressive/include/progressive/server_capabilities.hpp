#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_capabilities_validate(const std::string& input);
std::string server_capabilities_process(const std::string& input);
json server_capabilities_toJson(const std::string& input);

} // namespace progressive
