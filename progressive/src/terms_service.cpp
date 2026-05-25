#include "progressive/terms_service.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool terms_service_validate(const std::string& input) { return !input.empty(); }
std::string terms_service_process(const std::string& input) { return input; }
json terms_service_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
