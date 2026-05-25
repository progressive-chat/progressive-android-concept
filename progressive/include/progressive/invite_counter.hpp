#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool invite_counter_validate(const std::string& input);
std::string invite_counter_process(const std::string& input);
json invite_counter_toJson(const std::string& input);

} // namespace progressive
