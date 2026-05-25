#include "progressive/web_search.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool web_search_validate(const std::string& input) { return !input.empty(); }
std::string web_search_process(const std::string& input) { return input; }
json web_search_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
