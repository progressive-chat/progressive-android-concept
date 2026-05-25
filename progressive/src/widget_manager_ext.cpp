#include "progressive/widget_manager_ext.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_manager_ext_validate(const std::string& input) { return !input.empty(); }
std::string widget_manager_ext_process(const std::string& input) { return input; }
json widget_manager_ext_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
