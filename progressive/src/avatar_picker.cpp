#include "progressive/avatar_picker.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_picker_validate(const std::string& input) { return !input.empty(); }
std::string avatar_picker_process(const std::string& input) { return input; }
json avatar_picker_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
