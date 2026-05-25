#include "progressive/spellcheck_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool spellcheck_utils_validate(const std::string& input) { return !input.empty(); }
std::string spellcheck_utils_process(const std::string& input) { return input; }
json spellcheck_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
