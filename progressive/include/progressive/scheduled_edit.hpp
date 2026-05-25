#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool scheduled_edit_validate(const std::string& input);
std::string scheduled_edit_process(const std::string& input);
json scheduled_edit_toJson(const std::string& input);

} // namespace progressive
