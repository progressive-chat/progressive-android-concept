#include "progressive/read_receipts_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool read_receipts_utils_validate(const std::string& input) { return !input.empty(); }
std::string read_receipts_utils_process(const std::string& input) { return input; }
json read_receipts_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
