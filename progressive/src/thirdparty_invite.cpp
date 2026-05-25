#include "progressive/thirdparty_invite.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thirdparty_invite_validate(const std::string& input) { return !input.empty(); }
std::string thirdparty_invite_process(const std::string& input) { return input; }
json thirdparty_invite_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
