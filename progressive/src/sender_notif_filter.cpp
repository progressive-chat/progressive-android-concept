#include "progressive/sender_notif_filter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sender_notif_filter_validate(const std::string& input) { return !input.empty(); }
std::string sender_notif_filter_process(const std::string& input) { return input; }
json sender_notif_filter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
