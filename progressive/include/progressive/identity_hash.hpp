#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool identity_hash_validate(const std::string& input);
std::string identity_hash_process(const std::string& input);
json identity_hash_toJson(const std::string& input);

} // namespace progressive
