#include "progressive/notification_cleaner.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool notification_cleaner_validate(const std::string& input) { return !input.empty(); }
std::string notification_cleaner_process(const std::string& input) { return input; }
json notification_cleaner_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
