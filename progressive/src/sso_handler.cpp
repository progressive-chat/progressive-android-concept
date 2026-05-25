#include "progressive/sso_handler.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sso_handler_validate(const std::string& input) { return !input.empty(); }
std::string sso_handler_process(const std::string& input) { return input; }
json sso_handler_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
