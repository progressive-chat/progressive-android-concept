#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool background_scheduler_validate(const std::string& input);
std::string background_scheduler_process(const std::string& input);
json background_scheduler_toJson(const std::string& input);

} // namespace progressive
