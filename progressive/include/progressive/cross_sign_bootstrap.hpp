#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cross_sign_bootstrap_validate(const std::string& input);
std::string cross_sign_bootstrap_process(const std::string& input);
json cross_sign_bootstrap_toJson(const std::string& input);

} // namespace progressive
