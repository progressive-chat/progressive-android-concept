#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool llm_validate(const std::string& input);
std::string llm_process(const std::string& input);
json llm_toJson(const std::string& input);

} // namespace progressive
