#include "progressive/account_export.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool account_export_validate(const std::string& input) { return !input.empty(); }
std::string account_export_process(const std::string& input) { return input; }
json account_export_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
