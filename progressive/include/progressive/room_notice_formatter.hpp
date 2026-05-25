#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_notice_formatter_validate(const std::string& input);
std::string room_notice_formatter_process(const std::string& input);
json room_notice_formatter_toJson(const std::string& input);

} // namespace progressive
