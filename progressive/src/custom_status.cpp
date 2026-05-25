#include "progressive/custom_status.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool custom_status_validate(const std::string& input) { return !input.empty(); }
std::string custom_status_process(const std::string& input) { return input; }
json custom_status_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
