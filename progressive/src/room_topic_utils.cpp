#include "progressive/room_topic_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_topic_utils_validate(const std::string& input) { return !input.empty(); }
std::string room_topic_utils_process(const std::string& input) { return input; }
json room_topic_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
