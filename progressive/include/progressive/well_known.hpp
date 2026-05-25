#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool well_known_validate(const std::string& input);
std::string well_known_process(const std::string& input);
json well_known_toJson(const std::string& input);

} // namespace progressive
