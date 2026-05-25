#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool timeline_state_utils_validate(const std::string& input);
std::string timeline_state_utils_process(const std::string& input);
json timeline_state_utils_toJson(const std::string& input);

} // namespace progressive
