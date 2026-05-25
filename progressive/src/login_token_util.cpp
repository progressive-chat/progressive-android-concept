#include "progressive/login_token_util.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool login_token_util_validate(const std::string& input) { return !input.empty(); }
std::string login_token_util_process(const std::string& input) { return input; }
json login_token_util_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
