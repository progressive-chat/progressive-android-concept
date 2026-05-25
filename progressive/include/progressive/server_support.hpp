#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_support_validate(const std::string& input);
std::string server_support_process(const std::string& input);
json server_support_toJson(const std::string& input);

} // namespace progressive
