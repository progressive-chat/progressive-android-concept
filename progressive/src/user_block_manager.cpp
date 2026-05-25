#include "progressive/user_block_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool user_block_manager_validate(const std::string& input) { return !input.empty(); }
std::string user_block_manager_process(const std::string& input) { return input; }
json user_block_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
