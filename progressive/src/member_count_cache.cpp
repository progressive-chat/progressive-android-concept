#include "progressive/member_count_cache.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool member_count_cache_validate(const std::string& input) { return !input.empty(); }
std::string member_count_cache_process(const std::string& input) { return input; }
json member_count_cache_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
