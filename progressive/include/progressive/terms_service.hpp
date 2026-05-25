#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool terms_service_validate(const std::string& input);
std::string terms_service_process(const std::string& input);
json terms_service_toJson(const std::string& input);

} // namespace progressive
