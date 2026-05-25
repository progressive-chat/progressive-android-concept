#include "progressive/media_progress_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_progress_utils_validate(const std::string& input) { return !input.empty(); }
std::string media_progress_utils_process(const std::string& input) { return input; }
json media_progress_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
