#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thirdparty_invite_validate(const std::string& input);
std::string thirdparty_invite_process(const std::string& input);
json thirdparty_invite_toJson(const std::string& input);

} // namespace progressive
