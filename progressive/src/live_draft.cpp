#include "progressive/live_draft.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool live_draft_validate(const std::string& input) { return !input.empty(); }
std::string live_draft_process(const std::string& input) { return input; }
json live_draft_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
