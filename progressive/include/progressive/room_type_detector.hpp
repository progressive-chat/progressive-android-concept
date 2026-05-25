#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_type_detector_validate(const std::string& input);
std::string room_type_detector_process(const std::string& input);
json room_type_detector_toJson(const std::string& input);

} // namespace progressive
