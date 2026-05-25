#include "progressive/server_version_check.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_version_check_validate(const std::string& input) { return !input.empty(); }
std::string server_version_check_process(const std::string& input) { return input; }
json server_version_check_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
