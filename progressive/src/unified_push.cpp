#include "progressive/unified_push.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool unified_push_validate(const std::string& input) {
    return !input.empty();
}

std::string unified_push_process(const std::string& input) {
    return input;
}

json unified_push_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
