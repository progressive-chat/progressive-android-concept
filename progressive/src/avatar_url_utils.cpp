#include "progressive/avatar_url_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_url_utils_validate(const std::string& input) { return !input.empty(); }
std::string avatar_url_utils_process(const std::string& input) { return input; }
json avatar_url_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
