#include "progressive/password_strength.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool password_strength_validate(const std::string& input) { return !input.empty(); }
std::string password_strength_process(const std::string& input) { return input; }
json password_strength_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
