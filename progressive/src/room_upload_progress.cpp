#include "progressive/room_upload_progress.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_upload_progress_validate(const std::string& input) {
    return !input.empty();
}

std::string room_upload_progress_process(const std::string& input) {
    return input;
}

json room_upload_progress_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
