#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_mask_validate(const std::string& input);
std::string user_mask_process(const std::string& input);
json user_mask_toJson(const std::string& input);

} // namespace progressive
