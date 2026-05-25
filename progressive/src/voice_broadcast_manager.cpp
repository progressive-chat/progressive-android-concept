#include "progressive/voice_broadcast_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool voice_broadcast_manager_validate(const std::string& input) { return !input.empty(); }
std::string voice_broadcast_manager_process(const std::string& input) { return input; }
json voice_broadcast_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
