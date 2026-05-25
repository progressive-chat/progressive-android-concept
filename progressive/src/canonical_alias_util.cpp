#include "progressive/canonical_alias_util.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool canonical_alias_util_validate(const std::string& input) { return !input.empty(); }
std::string canonical_alias_util_process(const std::string& input) { return input; }
json canonical_alias_util_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
