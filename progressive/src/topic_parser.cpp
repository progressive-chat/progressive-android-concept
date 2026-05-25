#include "progressive/topic_parser.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool topic_parser_validate(const std::string& input) { return !input.empty(); }
std::string topic_parser_process(const std::string& input) { return input; }
json topic_parser_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
