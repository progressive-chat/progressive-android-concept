#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_directory_formatter_validate(const std::string& input);
std::string user_directory_formatter_process(const std::string& input);
json user_directory_formatter_toJson(const std::string& input);

} // namespace progressive
