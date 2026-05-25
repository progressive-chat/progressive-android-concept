#include "progressive/secret_storage_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool secret_storage_utils_validate(const std::string& input) { return !input.empty(); }
std::string secret_storage_utils_process(const std::string& input) { return input; }
json secret_storage_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
