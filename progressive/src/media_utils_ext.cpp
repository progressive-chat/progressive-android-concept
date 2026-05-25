#include "progressive/media_utils_ext.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_utils_ext_validate(const std::string& input) { return !input.empty(); }
std::string media_utils_ext_process(const std::string& input) { return input; }
json media_utils_ext_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
