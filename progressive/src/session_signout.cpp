#include "progressive/session_signout.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool session_signout_validate(const std::string& input) { return !input.empty(); }
std::string session_signout_process(const std::string& input) { return input; }
json session_signout_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
