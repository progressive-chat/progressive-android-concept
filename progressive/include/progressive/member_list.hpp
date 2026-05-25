#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool member_list_validate(const std::string& input);
std::string member_list_process(const std::string& input);
json member_list_toJson(const std::string& input);

} // namespace progressive
