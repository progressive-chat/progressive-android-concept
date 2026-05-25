#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_manager_full_validate(const std::string& input);
std::string device_manager_full_process(const std::string& input);
json device_manager_full_toJson(const std::string& input);

} // namespace progressive
