#include "progressive/message_encryption.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_encryption_validate(const std::string& input) { return !input.empty(); }
std::string message_encryption_process(const std::string& input) { return input; }
json message_encryption_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
