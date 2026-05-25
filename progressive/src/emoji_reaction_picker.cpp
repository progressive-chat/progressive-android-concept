#include "progressive/emoji_reaction_picker.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool emoji_reaction_picker_validate(const std::string& input) { return !input.empty(); }
std::string emoji_reaction_picker_process(const std::string& input) { return input; }
json emoji_reaction_picker_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
