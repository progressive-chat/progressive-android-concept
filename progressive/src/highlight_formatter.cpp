#include "progressive/highlight_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool highlight_formatter_validate(const std::string& input) { return !input.empty(); }
std::string highlight_formatter_process(const std::string& input) { return input; }
json highlight_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
