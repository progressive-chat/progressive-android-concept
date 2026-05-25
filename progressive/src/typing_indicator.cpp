#include "progressive/typing_indicator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool typing_indicator_validate(const std::string& input) { return !input.empty(); }
std::string typing_indicator_process(const std::string& input) { return input; }
json typing_indicator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
