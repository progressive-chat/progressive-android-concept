#include "progressive/sliding_sync.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sliding_sync_validate(const std::string& input) { return !input.empty(); }
std::string sliding_sync_process(const std::string& input) { return input; }
json sliding_sync_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
