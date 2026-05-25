#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_gallery_validate(const std::string& input);
std::string media_gallery_process(const std::string& input);
json media_gallery_toJson(const std::string& input);

} // namespace progressive
