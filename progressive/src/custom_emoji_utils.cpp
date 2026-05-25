#include "progressive/custom_emoji_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool custom_emoji_utils_validate(const std::string& input) { return !input.empty(); }
std::string custom_emoji_utils_process(const std::string& input) { return input; }
json custom_emoji_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
