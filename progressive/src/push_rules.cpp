#include "progressive/push_rules.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_rules_validate(const std::string& input) {
    return !input.empty();
}

std::string push_rules_process(const std::string& input) {
    return input;
}

json push_rules_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
