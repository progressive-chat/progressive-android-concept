#include "progressive/key_share.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool key_share_validate(const std::string& input) { return !input.empty(); }
std::string key_share_process(const std::string& input) { return input; }
json key_share_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
