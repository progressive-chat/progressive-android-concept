#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_autocomplete_validate(const std::string& input);
std::string room_autocomplete_process(const std::string& input);
json room_autocomplete_toJson(const std::string& input);

} // namespace progressive
