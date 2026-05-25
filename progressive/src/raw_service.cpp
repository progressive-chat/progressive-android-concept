#include "progressive/raw_service.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool raw_service_validate(const std::string& input) { return !input.empty(); }
std::string raw_service_process(const std::string& input) { return input; }
json raw_service_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
