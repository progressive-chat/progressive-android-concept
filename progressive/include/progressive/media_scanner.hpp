#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_scanner_validate(const std::string& input);
std::string media_scanner_process(const std::string& input);
json media_scanner_toJson(const std::string& input);

} // namespace progressive
