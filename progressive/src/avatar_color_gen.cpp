#include "progressive/avatar_color_gen.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_color_gen_validate(const std::string& input) { return !input.empty(); }
std::string avatar_color_gen_process(const std::string& input) { return input; }
json avatar_color_gen_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
