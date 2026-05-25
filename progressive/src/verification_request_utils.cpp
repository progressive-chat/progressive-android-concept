#include "progressive/verification_request_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool verification_request_utils_validate(const std::string& input) { return !input.empty(); }
std::string verification_request_utils_process(const std::string& input) { return input; }
json verification_request_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
