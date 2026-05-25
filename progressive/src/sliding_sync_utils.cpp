#include "progressive/sliding_sync_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sliding_sync_utils_validate(const std::string& input) { return !input.empty(); }
std::string sliding_sync_utils_process(const std::string& input) { return input; }
json sliding_sync_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
