#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool raw_service_validate(const std::string& input);
std::string raw_service_process(const std::string& input);
json raw_service_toJson(const std::string& input);

} // namespace progressive
