#include "progressive/power_level_visualizer.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool power_level_visualizer_validate(const std::string& input) { return !input.empty(); }
std::string power_level_visualizer_process(const std::string& input) { return input; }
json power_level_visualizer_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
