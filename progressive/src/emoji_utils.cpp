#include "progressive/emoji_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_utils_validate(const std::string& input) { return !input.empty(); }
std::string emoji_utils_process(const std::string& input) { return input; }
json emoji_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
