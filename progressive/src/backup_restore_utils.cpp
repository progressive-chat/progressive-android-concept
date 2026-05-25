#include "progressive/backup_restore_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool backup_restore_utils_validate(const std::string& input) { return !input.empty(); }
std::string backup_restore_utils_process(const std::string& input) { return input; }
json backup_restore_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
