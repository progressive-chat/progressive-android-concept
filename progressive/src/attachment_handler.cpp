#include "progressive/attachment_handler.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool attachment_handler_validate(const std::string& input) { return !input.empty(); }
std::string attachment_handler_process(const std::string& input) { return input; }
json attachment_handler_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
