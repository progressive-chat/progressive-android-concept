#include "progressive/dispatch_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool dispatch_manager_validate(const std::string& input) { return !input.empty(); }
std::string dispatch_manager_process(const std::string& input) { return input; }
json dispatch_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
