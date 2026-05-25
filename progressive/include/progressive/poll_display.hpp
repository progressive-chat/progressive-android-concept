#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool poll_display_validate(const std::string& input);
std::string poll_display_process(const std::string& input);
json poll_display_toJson(const std::string& input);

} // namespace progressive
