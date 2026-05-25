#include "progressive/rate_limit_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool rate_limit_utils_validate(const std::string& input) { return !input.empty(); }
std::string rate_limit_utils_process(const std::string& input) { return input; }
json rate_limit_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
