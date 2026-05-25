#include "progressive/url_preview_parser.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool url_preview_parser_validate(const std::string& input) { return !input.empty(); }
std::string url_preview_parser_process(const std::string& input) { return input; }
json url_preview_parser_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
