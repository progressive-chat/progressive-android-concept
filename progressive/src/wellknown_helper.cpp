#include "progressive/wellknown_helper.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool wellknown_helper_validate(const std::string& input) { return !input.empty(); }
std::string wellknown_helper_process(const std::string& input) { return input; }
json wellknown_helper_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
