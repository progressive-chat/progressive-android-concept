#include "progressive/read_receipt_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool read_receipt_formatter_validate(const std::string& input) { return !input.empty(); }
std::string read_receipt_formatter_process(const std::string& input) { return input; }
json read_receipt_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
