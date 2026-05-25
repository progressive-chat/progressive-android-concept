#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_pagination_validate(const std::string& input);
std::string event_pagination_process(const std::string& input);
json event_pagination_toJson(const std::string& input);

} // namespace progressive
