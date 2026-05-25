#include "progressive/sso_provider_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sso_provider_utils_validate(const std::string& input) { return !input.empty(); }
std::string sso_provider_utils_process(const std::string& input) { return input; }
json sso_provider_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
