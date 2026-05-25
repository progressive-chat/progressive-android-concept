#include "progressive/session_restore.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_restore_validate(const std::string& input) { return !input.empty(); }
std::string session_restore_process(const std::string& input) { return input; }
json session_restore_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
