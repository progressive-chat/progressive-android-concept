#include "progressive/room_notice_formatter.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool room_notice_formatter_validate(const std::string& input) { return !input.empty(); }
std::string room_notice_formatter_process(const std::string& input) { return input; }
json room_notice_formatter_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
