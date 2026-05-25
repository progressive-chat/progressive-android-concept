#include "progressive/megolm_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool megolm_utils_validate(const std::string& input) { return !input.empty(); }
std::string megolm_utils_process(const std::string& input) { return input; }
json megolm_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
