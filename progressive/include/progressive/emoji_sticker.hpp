#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_sticker_validate(const std::string& input);
std::string emoji_sticker_process(const std::string& input);
json emoji_sticker_toJson(const std::string& input);

} // namespace progressive
