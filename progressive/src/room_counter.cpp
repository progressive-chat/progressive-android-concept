#include "progressive/room_counter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_counter_validate(const std::string& input) {
    return !input.empty();
}

std::string room_counter_process(const std::string& input) {
    return input;
}

json room_counter_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
