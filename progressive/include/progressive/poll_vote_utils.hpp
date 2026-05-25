#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool poll_vote_utils_validate(const std::string& input);
std::string poll_vote_utils_process(const std::string& input);
json poll_vote_utils_toJson(const std::string& input);

} // namespace progressive
