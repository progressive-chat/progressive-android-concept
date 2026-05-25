#include "progressive/encryption_key_backup.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encryption_key_backup_validate(const std::string& input) { return !input.empty(); }
std::string encryption_key_backup_process(const std::string& input) { return input; }
json encryption_key_backup_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
