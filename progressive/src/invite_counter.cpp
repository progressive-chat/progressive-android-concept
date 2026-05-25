#include "progressive/invite_counter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool invite_counter_validate(const std::string& input) { return !input.empty(); }
std::string invite_counter_process(const std::string& input) { return input; }
json invite_counter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
