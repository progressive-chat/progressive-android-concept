#include "progressive/cache_invalidator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool cache_invalidator_validate(const std::string& input) { return !input.empty(); }
std::string cache_invalidator_process(const std::string& input) { return input; }
json cache_invalidator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
