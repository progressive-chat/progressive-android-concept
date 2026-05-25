#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_reaction_picker_validate(const std::string& input);
std::string emoji_reaction_picker_process(const std::string& input);
json emoji_reaction_picker_toJson(const std::string& input);

} // namespace progressive
