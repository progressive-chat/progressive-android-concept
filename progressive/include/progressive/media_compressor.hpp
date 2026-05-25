#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_compressor_validate(const std::string& input);
std::string media_compressor_process(const std::string& input);
json media_compressor_toJson(const std::string& input);

} // namespace progressive
