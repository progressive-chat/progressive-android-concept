#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool wellknown_helper_validate(const std::string& input);
std::string wellknown_helper_process(const std::string& input);
json wellknown_helper_toJson(const std::string& input);

} // namespace progressive
