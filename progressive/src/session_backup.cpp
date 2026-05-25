#include "progressive/session_backup.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_backup_validate(const std::string& input) { return !input.empty(); }
std::string session_backup_process(const std::string& input) { return input; }
json session_backup_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
