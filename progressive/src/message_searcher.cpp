#include "progressive/message_searcher.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool message_searcher_validate(const std::string& input) { return !input.empty(); }
std::string message_searcher_process(const std::string& input) { return input; }
json message_searcher_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
