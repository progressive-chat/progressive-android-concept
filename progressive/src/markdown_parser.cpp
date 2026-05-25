#include "progressive/markdown_parser.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool markdown_parser_validate(const std::string& input) { return !input.empty(); }
std::string markdown_parser_process(const std::string& input) { return input; }
json markdown_parser_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
