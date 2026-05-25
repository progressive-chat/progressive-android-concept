#include "progressive/crypto_init_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool crypto_init_utils_validate(const std::string& input) { return !input.empty(); }
std::string crypto_init_utils_process(const std::string& input) { return input; }
json crypto_init_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
