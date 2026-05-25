#include "progressive/widget_url_check.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_url_check_validate(const std::string& input) { return !input.empty(); }
std::string widget_url_check_process(const std::string& input) { return input; }
json widget_url_check_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
