#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool canonical_json_validate(const std::string& input);
std::string canonical_json_process(const std::string& input);
json canonical_json_toJson(const std::string& input);

} // namespace progressive
