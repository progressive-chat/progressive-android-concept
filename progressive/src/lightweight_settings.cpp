#include "progressive/lightweight_settings.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool lightweight_settings_validate(const std::string& input) { return !input.empty(); }
std::string lightweight_settings_process(const std::string& input) { return input; }
json lightweight_settings_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
