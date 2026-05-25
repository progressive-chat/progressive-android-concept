#include "progressive/font_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool font_manager_validate(const std::string& input) { return !input.empty(); }
std::string font_manager_process(const std::string& input) { return input; }
json font_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
