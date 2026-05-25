#include "progressive/reaction_aggregator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool reaction_aggregator_validate(const std::string& input) { return !input.empty(); }
std::string reaction_aggregator_process(const std::string& input) { return input; }
json reaction_aggregator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
