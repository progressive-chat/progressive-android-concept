#include "progressive/power_levels.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool power_levels_validate(const std::string& input) {
    return !input.empty();
}

std::string power_levels_process(const std::string& input) {
    return input;
}

json power_levels_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
