#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool registration_flow_validate(const std::string& input);
std::string registration_flow_process(const std::string& input);
json registration_flow_toJson(const std::string& input);

} // namespace progressive
