#include "progressive/device_verifier.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool device_verifier_validate(const std::string& input) { return !input.empty(); }
std::string device_verifier_process(const std::string& input) { return input; }
json device_verifier_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
