#include "progressive/deeplink_parser.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool deeplink_parser_validate(const std::string& input) { return !input.empty(); }
std::string deeplink_parser_process(const std::string& input) { return input; }
json deeplink_parser_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
