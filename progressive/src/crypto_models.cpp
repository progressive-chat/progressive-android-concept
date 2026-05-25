#include "progressive/crypto_models.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool crypto_models_validate(const std::string& input) {
    return !input.empty();
}

std::string crypto_models_process(const std::string& input) {
    return input;
}

json crypto_models_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
