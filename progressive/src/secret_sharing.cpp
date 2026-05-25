#include "progressive/secret_sharing.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool secret_sharing_validate(const std::string& input) { return !input.empty(); }
std::string secret_sharing_process(const std::string& input) { return input; }
json secret_sharing_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
