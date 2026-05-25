#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_search_validate(const std::string& input);
std::string message_search_process(const std::string& input);
json message_search_toJson(const std::string& input);

} // namespace progressive
