#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool language_switcher_validate(const std::string& input);
std::string language_switcher_process(const std::string& input);
json language_switcher_toJson(const std::string& input);

} // namespace progressive
