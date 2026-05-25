#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool identity_lookup_validate(const std::string& input);
std::string identity_lookup_process(const std::string& input);
json identity_lookup_toJson(const std::string& input);

} // namespace progressive
