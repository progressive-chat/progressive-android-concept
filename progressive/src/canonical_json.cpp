#include "progressive/canonical_json.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool canonical_json_validate(const std::string& input) { return !input.empty(); }
std::string canonical_json_process(const std::string& input) { return input; }
json canonical_json_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
