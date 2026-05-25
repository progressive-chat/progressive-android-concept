#include "progressive/key_backup_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool key_backup_manager_validate(const std::string& input) {
    return !input.empty();
}

std::string key_backup_manager_process(const std::string& input) {
    return input;
}

json key_backup_manager_toJson(const std::string& input) {
    json j;
    j["value"] = input;
    return j;
}

} // namespace progressive
