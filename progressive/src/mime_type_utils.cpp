#include "progressive/mime_type_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool mime_type_utils_validate(const std::string& input) { return !input.empty(); }
std::string mime_type_utils_process(const std::string& input) { return input; }
json mime_type_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
