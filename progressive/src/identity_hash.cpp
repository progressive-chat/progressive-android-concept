#include "progressive/identity_hash.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool identity_hash_validate(const std::string& input) { return !input.empty(); }
std::string identity_hash_process(const std::string& input) { return input; }
json identity_hash_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
