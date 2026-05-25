#include "progressive/emoji_search.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_search_validate(const std::string& input) { return !input.empty(); }
std::string emoji_search_process(const std::string& input) { return input; }
json emoji_search_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
