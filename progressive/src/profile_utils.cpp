#include "progressive/profile_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool profile_utils_validate(const std::string& input) { return !input.empty(); }
std::string profile_utils_process(const std::string& input) { return input; }
json profile_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
