#include "progressive/language_switcher.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool language_switcher_validate(const std::string& input) { return !input.empty(); }
std::string language_switcher_process(const std::string& input) { return input; }
json language_switcher_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
