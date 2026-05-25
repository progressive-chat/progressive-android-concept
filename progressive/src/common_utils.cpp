#include "progressive/common_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool common_utils_validate(const std::string& input) { return !input.empty(); }
std::string common_utils_process(const std::string& input) { return input; }
json common_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
