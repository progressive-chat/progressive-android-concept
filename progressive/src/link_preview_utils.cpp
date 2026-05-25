#include "progressive/link_preview_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool link_preview_utils_validate(const std::string& input) { return !input.empty(); }
std::string link_preview_utils_process(const std::string& input) { return input; }
json link_preview_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
