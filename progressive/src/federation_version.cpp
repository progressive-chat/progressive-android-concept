#include "progressive/federation_version.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool federation_version_validate(const std::string& input) { return !input.empty(); }
std::string federation_version_process(const std::string& input) { return input; }
json federation_version_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
