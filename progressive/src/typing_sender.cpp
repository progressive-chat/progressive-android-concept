#include "progressive/typing_sender.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool typing_sender_validate(const std::string& input) { return !input.empty(); }
std::string typing_sender_process(const std::string& input) { return input; }
json typing_sender_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
