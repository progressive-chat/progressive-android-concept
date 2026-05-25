#include "progressive/location_share.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool location_share_validate(const std::string& input) { return !input.empty(); }
std::string location_share_process(const std::string& input) { return input; }
json location_share_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
