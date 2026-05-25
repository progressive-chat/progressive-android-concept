#include "progressive/poll_display.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool poll_display_validate(const std::string& input) { return !input.empty(); }
std::string poll_display_process(const std::string& input) { return input; }
json poll_display_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
