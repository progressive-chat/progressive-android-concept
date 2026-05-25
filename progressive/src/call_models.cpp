#include "progressive/call_models.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool call_models_validate(const std::string& input) { return !input.empty(); }
std::string call_models_process(const std::string& input) { return input; }
json call_models_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
