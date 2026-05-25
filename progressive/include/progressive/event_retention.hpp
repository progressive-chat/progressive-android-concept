#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_retention_validate(const std::string& input);
std::string event_retention_process(const std::string& input);
json event_retention_toJson(const std::string& input);

} // namespace progressive
