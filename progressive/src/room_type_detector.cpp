#include "progressive/room_type_detector.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_type_detector_validate(const std::string& input) { return !input.empty(); }
std::string room_type_detector_process(const std::string& input) { return input; }
json room_type_detector_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
