#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool url_preview_formatter_validate(const std::string& input);
std::string url_preview_formatter_process(const std::string& input);
json url_preview_formatter_toJson(const std::string& input);

} // namespace progressive
