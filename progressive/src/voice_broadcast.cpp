#include "progressive/voice_broadcast.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool voice_broadcast_validate(const std::string& input) {
    return !input.empty();
}

std::string voice_broadcast_process(const std::string& input) {
    return input;
}

json voice_broadcast_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
