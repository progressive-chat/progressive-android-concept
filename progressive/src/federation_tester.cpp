#include "progressive/federation_tester.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool federation_tester_validate(const std::string& input) { return !input.empty(); }
std::string federation_tester_process(const std::string& input) { return input; }
json federation_tester_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
