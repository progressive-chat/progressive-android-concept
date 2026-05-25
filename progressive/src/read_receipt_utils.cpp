#include "progressive/read_receipt_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool read_receipt_utils_validate(const std::string& input) { return !input.empty(); }
std::string read_receipt_utils_process(const std::string& input) { return input; }
json read_receipt_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
