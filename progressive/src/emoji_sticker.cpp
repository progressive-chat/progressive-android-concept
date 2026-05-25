#include "progressive/emoji_sticker.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_sticker_validate(const std::string& input) { return !input.empty(); }
std::string emoji_sticker_process(const std::string& input) { return input; }
json emoji_sticker_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
