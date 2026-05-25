#include "progressive/cross_signing_status.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cross_signing_status_validate(const std::string& input) { return !input.empty(); }
std::string cross_signing_status_process(const std::string& input) { return input; }
json cross_signing_status_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
