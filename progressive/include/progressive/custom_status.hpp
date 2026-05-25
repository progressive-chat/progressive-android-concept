#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool custom_status_validate(const std::string& input);
std::string custom_status_process(const std::string& input);
json custom_status_toJson(const std::string& input);

} // namespace progressive
