#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool crash_reporter_validate(const std::string& input);
std::string crash_reporter_process(const std::string& input);
json crash_reporter_toJson(const std::string& input);

} // namespace progressive
