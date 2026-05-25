#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool federation_version_validate(const std::string& input);
std::string federation_version_process(const std::string& input);
json federation_version_toJson(const std::string& input);

} // namespace progressive
