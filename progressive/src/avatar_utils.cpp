#include "progressive/avatar_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_utils_validate(const std::string& input) { return !input.empty(); }
std::string avatar_utils_process(const std::string& input) { return input; }
json avatar_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
