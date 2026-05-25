#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool read_tracker_validate(const std::string& input);
std::string read_tracker_process(const std::string& input);
json read_tracker_toJson(const std::string& input);

} // namespace progressive
