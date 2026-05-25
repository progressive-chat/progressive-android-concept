#include "progressive/url_preview_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool url_preview_formatter_validate(const std::string& input) { return !input.empty(); }
std::string url_preview_formatter_process(const std::string& input) { return input; }
json url_preview_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
