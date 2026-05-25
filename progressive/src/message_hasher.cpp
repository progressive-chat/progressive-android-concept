#include "progressive/message_hasher.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_hasher_validate(const std::string& input) { return !input.empty(); }
std::string message_hasher_process(const std::string& input) { return input; }
json message_hasher_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
