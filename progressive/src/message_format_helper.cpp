#include "progressive/message_format_helper.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_format_helper_validate(const std::string& input) { return !input.empty(); }
std::string message_format_helper_process(const std::string& input) { return input; }
json message_format_helper_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
