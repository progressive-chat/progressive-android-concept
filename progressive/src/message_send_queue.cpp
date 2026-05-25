#include "progressive/message_send_queue.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_send_queue_validate(const std::string& input) { return !input.empty(); }
std::string message_send_queue_process(const std::string& input) { return input; }
json message_send_queue_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
