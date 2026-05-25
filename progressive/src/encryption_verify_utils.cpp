#include "progressive/encryption_verify_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encryption_verify_utils_validate(const std::string& input) { return !input.empty(); }
std::string encryption_verify_utils_process(const std::string& input) { return input; }
json encryption_verify_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
