#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool reaction_sync_validate(const std::string& input);
std::string reaction_sync_process(const std::string& input);
json reaction_sync_toJson(const std::string& input);

} // namespace progressive
