#include "progressive/notif_mode.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notif_mode_validate(const std::string& input) { return !input.empty(); }
std::string notif_mode_process(const std::string& input) { return input; }
json notif_mode_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
