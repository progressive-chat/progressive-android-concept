#include "progressive/auth_models.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool auth_models_validate(const std::string& input) {
    return !input.empty();
}

std::string auth_models_process(const std::string& input) {
    return input;
}

json auth_models_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive

namespace progressive {
Credentials parseCredentials(const std::string& json) { Credentials c; return c; }
} // namespace progressive
