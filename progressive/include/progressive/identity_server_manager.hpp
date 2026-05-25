#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool identity_server_manager_validate(const std::string& input);
std::string identity_server_manager_process(const std::string& input);
json identity_server_manager_toJson(const std::string& input);

} // namespace progressive
