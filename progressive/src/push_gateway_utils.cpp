#include "progressive/push_gateway_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_gateway_utils_validate(const std::string& input) { return !input.empty(); }
std::string push_gateway_utils_process(const std::string& input) { return input; }
json push_gateway_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
