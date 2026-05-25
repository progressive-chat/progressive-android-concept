#include "progressive/favorite_messages.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool favorite_messages_validate(const std::string& input) { return !input.empty(); }
std::string favorite_messages_process(const std::string& input) { return input; }
json favorite_messages_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
