#include "progressive/spotlight_search_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool spotlight_search_utils_validate(const std::string& input) { return !input.empty(); }
std::string spotlight_search_utils_process(const std::string& input) { return input; }
json spotlight_search_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
