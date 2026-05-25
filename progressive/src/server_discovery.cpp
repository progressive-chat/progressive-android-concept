#include "progressive/server_discovery.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_discovery_validate(const std::string& input) { return !input.empty(); }
std::string server_discovery_process(const std::string& input) { return input; }
json server_discovery_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
