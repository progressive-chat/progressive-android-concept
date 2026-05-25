#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_discovery_validate(const std::string& input);
std::string server_discovery_process(const std::string& input);
json server_discovery_toJson(const std::string& input);

} // namespace progressive
