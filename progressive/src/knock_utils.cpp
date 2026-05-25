#include "progressive/knock_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool knock_utils_validate(const std::string& input) { return !input.empty(); }
std::string knock_utils_process(const std::string& input) { return input; }
json knock_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
