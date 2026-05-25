#include "progressive/device_trust.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_trust_validate(const std::string& input) { return !input.empty(); }
std::string device_trust_process(const std::string& input) { return input; }
json device_trust_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
