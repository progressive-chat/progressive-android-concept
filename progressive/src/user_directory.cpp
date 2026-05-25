#include "progressive/user_directory.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_directory_validate(const std::string& input) { return !input.empty(); }
std::string user_directory_process(const std::string& input) { return input; }
json user_directory_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
