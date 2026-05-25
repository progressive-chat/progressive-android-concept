#include "progressive/user_directory_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_directory_formatter_validate(const std::string& input) { return !input.empty(); }
std::string user_directory_formatter_process(const std::string& input) { return input; }
json user_directory_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
