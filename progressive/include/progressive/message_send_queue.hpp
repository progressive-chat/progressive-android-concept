#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_send_queue_validate(const std::string& input);
std::string message_send_queue_process(const std::string& input);
json message_send_queue_toJson(const std::string& input);

} // namespace progressive
