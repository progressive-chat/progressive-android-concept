#include "progressive/crash_reporter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool crash_reporter_validate(const std::string& input) { return !input.empty(); }
std::string crash_reporter_process(const std::string& input) { return input; }
json crash_reporter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
