#include "progressive/session_export.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_export_validate(const std::string& input) { return !input.empty(); }
std::string session_export_process(const std::string& input) { return input; }
json session_export_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
