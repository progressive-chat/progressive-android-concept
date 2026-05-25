#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool federation_tester_validate(const std::string& input);
std::string federation_tester_process(const std::string& input);
json federation_tester_toJson(const std::string& input);

} // namespace progressive
