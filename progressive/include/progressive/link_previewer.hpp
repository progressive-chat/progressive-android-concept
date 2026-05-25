#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool link_previewer_validate(const std::string& input);
std::string link_previewer_process(const std::string& input);
json link_previewer_toJson(const std::string& input);

} // namespace progressive
