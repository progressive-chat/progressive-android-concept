#include "progressive/content_warning_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool content_warning_utils_validate(const std::string& input) { return !input.empty(); }
std::string content_warning_utils_process(const std::string& input) { return input; }
json content_warning_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
