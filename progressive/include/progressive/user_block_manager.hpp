#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_block_manager_validate(const std::string& input);
std::string user_block_manager_process(const std::string& input);
json user_block_manager_toJson(const std::string& input);

} // namespace progressive
