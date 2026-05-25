#include "progressive/pagination_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool pagination_utils_validate(const std::string& input) { return !input.empty(); }
std::string pagination_utils_process(const std::string& input) { return input; }
json pagination_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
