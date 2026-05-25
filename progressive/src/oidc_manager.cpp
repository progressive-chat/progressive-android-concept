#include "progressive/oidc_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool oidc_manager_validate(const std::string& input) { return !input.empty(); }
std::string oidc_manager_process(const std::string& input) { return input; }
json oidc_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
