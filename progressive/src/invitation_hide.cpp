#include "progressive/invitation_hide.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool invitation_hide_validate(const std::string& input) { return !input.empty(); }
std::string invitation_hide_process(const std::string& input) { return input; }
json invitation_hide_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
