#include "progressive/registration_flow.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool registration_flow_validate(const std::string& input) { return !input.empty(); }
std::string registration_flow_process(const std::string& input) { return input; }
json registration_flow_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
