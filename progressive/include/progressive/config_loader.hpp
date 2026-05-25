#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool config_loader_validate(const std::string& input);
std::string config_loader_process(const std::string& input);
json config_loader_toJson(const std::string& input);

} // namespace progressive
