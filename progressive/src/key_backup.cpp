#include "progressive/key_backup.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool key_backup_validate(const std::string& input) {
    return !input.empty();
}

std::string key_backup_process(const std::string& input) {
    return input;
}

json key_backup_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
