#include "progressive/lang_detect.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool lang_detect_validate(const std::string& input) { return !input.empty(); }
std::string lang_detect_process(const std::string& input) { return input; }
json lang_detect_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
