#include "progressive/reply_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool reply_utils_validate(const std::string& input) { return !input.empty(); }
std::string reply_utils_process(const std::string& input) { return input; }
json reply_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
