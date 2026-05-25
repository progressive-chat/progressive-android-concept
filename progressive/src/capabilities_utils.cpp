#include "progressive/capabilities_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool capabilities_utils_validate(const std::string& input) { return !input.empty(); }
std::string capabilities_utils_process(const std::string& input) { return input; }
json capabilities_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
