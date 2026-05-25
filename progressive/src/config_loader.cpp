#include "progressive/config_loader.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool config_loader_validate(const std::string& input) { return !input.empty(); }
std::string config_loader_process(const std::string& input) { return input; }
json config_loader_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
