#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool dispatch_manager_validate(const std::string& input);
std::string dispatch_manager_process(const std::string& input);
json dispatch_manager_toJson(const std::string& input);

} // namespace progressive
