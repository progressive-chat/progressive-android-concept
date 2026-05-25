#include "progressive/media_scanner.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_scanner_validate(const std::string& input) { return !input.empty(); }
std::string media_scanner_process(const std::string& input) { return input; }
json media_scanner_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
