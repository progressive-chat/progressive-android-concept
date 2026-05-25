#include "progressive/well_known.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool well_known_validate(const std::string& input) { return !input.empty(); }
std::string well_known_process(const std::string& input) { return input; }
json well_known_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
