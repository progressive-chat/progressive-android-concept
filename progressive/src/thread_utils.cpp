#include "progressive/thread_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thread_utils_validate(const std::string& input) { return !input.empty(); }
std::string thread_utils_process(const std::string& input) { return input; }
json thread_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
