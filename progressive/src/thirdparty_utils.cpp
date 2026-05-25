#include "progressive/thirdparty_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thirdparty_utils_validate(const std::string& input) { return !input.empty(); }
std::string thirdparty_utils_process(const std::string& input) { return input; }
json thirdparty_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
