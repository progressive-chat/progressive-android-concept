#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool spellcheck_utils_validate(const std::string& input);
std::string spellcheck_utils_process(const std::string& input);
json spellcheck_utils_toJson(const std::string& input);

} // namespace progressive
