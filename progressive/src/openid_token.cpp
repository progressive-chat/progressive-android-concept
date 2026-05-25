#include "progressive/openid_token.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool openid_token_validate(const std::string& input) { return !input.empty(); }
std::string openid_token_process(const std::string& input) { return input; }
json openid_token_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
