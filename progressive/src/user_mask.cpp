#include "progressive/user_mask.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_mask_validate(const std::string& input) { return !input.empty(); }
std::string user_mask_process(const std::string& input) { return input; }
json user_mask_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
