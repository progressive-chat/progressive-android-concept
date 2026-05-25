#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool verification_request_utils_validate(const std::string& input);
std::string verification_request_utils_process(const std::string& input);
json verification_request_utils_toJson(const std::string& input);

} // namespace progressive
