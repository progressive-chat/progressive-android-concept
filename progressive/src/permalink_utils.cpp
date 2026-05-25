#include "progressive/permalink_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool permalink_utils_validate(const std::string& input) { return !input.empty(); }
std::string permalink_utils_process(const std::string& input) { return input; }
json permalink_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
