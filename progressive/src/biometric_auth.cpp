#include "progressive/biometric_auth.hpp"
#include <string>
#include <nlohmann/json.hpp>

namespace progressive {
using json = nlohmann::json;
bool biometric_auth_validate(const std::string& input) { return !input.empty(); }
std::string biometric_auth_process(const std::string& input) { return input; }
json biometric_auth_toJson(const std::string& input) { return json::object(); }
} // namespace progressive
