#include "progressive/identity_server_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool identity_server_manager_validate(const std::string& input) { return !input.empty(); }
std::string identity_server_manager_process(const std::string& input) { return input; }
json identity_server_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
