#include "progressive/room_content.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_content_validate(const std::string& input) {
    return !input.empty();
}

std::string room_content_process(const std::string& input) {
    return input;
}

json room_content_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
