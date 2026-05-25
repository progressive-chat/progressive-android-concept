#include "progressive/voice_message_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool voice_message_utils_validate(const std::string& input) { return !input.empty(); }
std::string voice_message_utils_process(const std::string& input) { return input; }
json voice_message_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
