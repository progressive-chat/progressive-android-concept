#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool presence_sync_validate(const std::string& input);
std::string presence_sync_process(const std::string& input);
json presence_sync_toJson(const std::string& input);

} // namespace progressive
