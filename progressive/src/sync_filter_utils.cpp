#include "progressive/sync_filter_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sync_filter_utils_validate(const std::string& input) { return !input.empty(); }
std::string sync_filter_utils_process(const std::string& input) { return input; }
json sync_filter_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
