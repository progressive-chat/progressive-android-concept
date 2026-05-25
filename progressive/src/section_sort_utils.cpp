#include "progressive/section_sort_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool section_sort_utils_validate(const std::string& input) { return !input.empty(); }
std::string section_sort_utils_process(const std::string& input) { return input; }
json section_sort_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
