#include "progressive/cross_sign_bootstrap.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cross_sign_bootstrap_validate(const std::string& input) { return !input.empty(); }
std::string cross_sign_bootstrap_process(const std::string& input) { return input; }
json cross_sign_bootstrap_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
