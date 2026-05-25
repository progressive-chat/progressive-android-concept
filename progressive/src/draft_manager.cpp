#include "progressive/draft_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool draft_manager_validate(const std::string& input) { return !input.empty(); }
std::string draft_manager_process(const std::string& input) { return input; }
json draft_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
