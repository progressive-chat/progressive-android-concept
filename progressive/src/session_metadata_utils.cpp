#include "progressive/session_metadata_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_metadata_utils_validate(const std::string& input) { return !input.empty(); }
std::string session_metadata_utils_process(const std::string& input) { return input; }
json session_metadata_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
