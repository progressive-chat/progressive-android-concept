#include "progressive/terms_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool terms_manager_validate(const std::string& input) { return !input.empty(); }
std::string terms_manager_process(const std::string& input) { return input; }
json terms_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
