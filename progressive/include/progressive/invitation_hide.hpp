#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool invitation_hide_validate(const std::string& input);
std::string invitation_hide_process(const std::string& input);
json invitation_hide_toJson(const std::string& input);

} // namespace progressive
