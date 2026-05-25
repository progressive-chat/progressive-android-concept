#include "progressive/three_pid_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool three_pid_utils_validate(const std::string& input) { return !input.empty(); }
std::string three_pid_utils_process(const std::string& input) { return input; }
json three_pid_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
