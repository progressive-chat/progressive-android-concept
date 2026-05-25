#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool live_draft_validate(const std::string& input);
std::string live_draft_process(const std::string& input);
json live_draft_toJson(const std::string& input);

} // namespace progressive
