#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_thumbnailer_validate(const std::string& input);
std::string media_thumbnailer_process(const std::string& input);
json media_thumbnailer_toJson(const std::string& input);

} // namespace progressive
