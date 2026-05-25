#include "progressive/reaction_sync.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool reaction_sync_validate(const std::string& input) { return !input.empty(); }
std::string reaction_sync_process(const std::string& input) { return input; }
json reaction_sync_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
