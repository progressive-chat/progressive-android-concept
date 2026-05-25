#include "progressive/thread_summary.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thread_summary_validate(const std::string& input) { return !input.empty(); }
std::string thread_summary_process(const std::string& input) { return input; }
json thread_summary_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
