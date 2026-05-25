#include "progressive/lazy_loading_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool lazy_loading_utils_validate(const std::string& input) { return !input.empty(); }
std::string lazy_loading_utils_process(const std::string& input) { return input; }
json lazy_loading_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
