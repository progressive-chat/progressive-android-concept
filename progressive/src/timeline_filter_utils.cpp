#include "progressive/timeline_filter_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool timeline_filter_utils_validate(const std::string& input) { return !input.empty(); }
std::string timeline_filter_utils_process(const std::string& input) { return input; }
json timeline_filter_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
