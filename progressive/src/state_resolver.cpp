#include "progressive/state_resolver.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool state_resolver_validate(const std::string& input) { return !input.empty(); }
std::string state_resolver_process(const std::string& input) { return input; }
json state_resolver_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
