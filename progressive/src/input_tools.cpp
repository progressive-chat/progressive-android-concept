#include "progressive/input_tools.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool input_tools_validate(const std::string& input) { return !input.empty(); }
std::string input_tools_process(const std::string& input) { return input; }
json input_tools_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
