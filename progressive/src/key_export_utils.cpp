#include "progressive/key_export_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool key_export_utils_validate(const std::string& input) { return !input.empty(); }
std::string key_export_utils_process(const std::string& input) { return input; }
json key_export_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
