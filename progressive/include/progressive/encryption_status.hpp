#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encryption_status_validate(const std::string& input);
std::string encryption_status_process(const std::string& input);
json encryption_status_toJson(const std::string& input);

} // namespace progressive
