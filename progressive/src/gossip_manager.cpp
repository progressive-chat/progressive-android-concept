#include "progressive/gossip_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool gossip_manager_validate(const std::string& input) { return !input.empty(); }
std::string gossip_manager_process(const std::string& input) { return input; }
json gossip_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
