#include "progressive/server_capabilities.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_capabilities_validate(const std::string& input) { return !input.empty(); }
std::string server_capabilities_process(const std::string& input) { return input; }
json server_capabilities_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
