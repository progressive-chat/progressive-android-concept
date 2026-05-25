#include "progressive/room_mirror.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_mirror_validate(const std::string& input) {
    return !input.empty();
}

std::string room_mirror_process(const std::string& input) {
    return input;
}

json room_mirror_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
