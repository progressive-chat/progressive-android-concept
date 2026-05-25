#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool highlight_formatter_validate(const std::string& input);
std::string highlight_formatter_process(const std::string& input);
json highlight_formatter_toJson(const std::string& input);

} // namespace progressive
