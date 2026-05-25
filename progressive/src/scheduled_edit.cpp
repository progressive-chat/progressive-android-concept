#include "progressive/scheduled_edit.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool scheduled_edit_validate(const std::string& input) { return !input.empty(); }
std::string scheduled_edit_process(const std::string& input) { return input; }
json scheduled_edit_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
