#include "progressive/read_tracker.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool read_tracker_validate(const std::string& input) { return !input.empty(); }
std::string read_tracker_process(const std::string& input) { return input; }
json read_tracker_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
