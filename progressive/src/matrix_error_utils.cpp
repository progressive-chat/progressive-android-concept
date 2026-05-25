#include "progressive/matrix_error_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool matrix_error_utils_validate(const std::string& input) { return !input.empty(); }
std::string matrix_error_utils_process(const std::string& input) { return input; }
json matrix_error_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
