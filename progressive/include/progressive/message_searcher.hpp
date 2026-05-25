#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_searcher_validate(const std::string& input);
std::string message_searcher_process(const std::string& input);
json message_searcher_toJson(const std::string& input);

} // namespace progressive
