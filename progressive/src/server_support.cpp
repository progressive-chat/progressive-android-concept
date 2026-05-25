#include "progressive/server_support.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool server_support_validate(const std::string& input) { return !input.empty(); }
std::string server_support_process(const std::string& input) { return input; }
json server_support_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
