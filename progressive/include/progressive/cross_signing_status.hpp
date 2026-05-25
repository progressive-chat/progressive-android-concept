#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cross_signing_status_validate(const std::string& input);
std::string cross_signing_status_process(const std::string& input);
json cross_signing_status_toJson(const std::string& input);

} // namespace progressive
