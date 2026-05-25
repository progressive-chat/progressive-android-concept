#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_search_validate(const std::string& input);
std::string emoji_search_process(const std::string& input);
json emoji_search_toJson(const std::string& input);

} // namespace progressive
