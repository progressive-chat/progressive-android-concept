#include "progressive/alias_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool alias_manager_validate(const std::string& input) { return !input.empty(); }
std::string alias_manager_process(const std::string& input) { return input; }
json alias_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
