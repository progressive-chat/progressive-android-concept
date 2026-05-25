#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_search_validate(const std::string& input);
std::string event_search_process(const std::string& input);
json event_search_toJson(const std::string& input);

} // namespace progressive
