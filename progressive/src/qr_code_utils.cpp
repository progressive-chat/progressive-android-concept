#include "progressive/qr_code_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool qr_code_utils_validate(const std::string& input) { return !input.empty(); }
std::string qr_code_utils_process(const std::string& input) { return input; }
json qr_code_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
