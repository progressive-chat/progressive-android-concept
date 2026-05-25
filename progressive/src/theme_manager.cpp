#include "progressive/theme_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool theme_manager_validate(const std::string& input) { return !input.empty(); }
std::string theme_manager_process(const std::string& input) { return input; }
json theme_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
