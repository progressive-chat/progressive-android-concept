#include "progressive/location_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool location_utils_validate(const std::string& input) { return !input.empty(); }
std::string location_utils_process(const std::string& input) { return input; }
json location_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
