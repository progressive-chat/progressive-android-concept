#include "progressive/event_models.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_models_validate(const std::string& input) {
    return !input.empty();
}

std::string event_models_process(const std::string& input) {
    return input;
}

json event_models_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
