#include "progressive/openid_token_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool openid_token_utils_validate(const std::string& input) { return !input.empty(); }
std::string openid_token_utils_process(const std::string& input) { return input; }
json openid_token_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
