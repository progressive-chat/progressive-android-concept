#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool state_resolver_validate(const std::string& input);
std::string state_resolver_process(const std::string& input);
json state_resolver_toJson(const std::string& input);

} // namespace progressive
