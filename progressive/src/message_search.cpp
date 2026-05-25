#include "progressive/message_search.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_search_validate(const std::string& input) { return !input.empty(); }
std::string message_search_process(const std::string& input) { return input; }
json message_search_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
