#include "progressive/admin_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool admin_utils_validate(const std::string& input) { return !input.empty(); }
std::string admin_utils_process(const std::string& input) { return input; }
json admin_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
