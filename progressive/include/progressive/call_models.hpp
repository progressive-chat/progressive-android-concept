#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool call_models_validate(const std::string& input);
std::string call_models_process(const std::string& input);
json call_models_toJson(const std::string& input);

} // namespace progressive
