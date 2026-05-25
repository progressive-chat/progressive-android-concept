#include "progressive/member_list.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool member_list_validate(const std::string& input) { return !input.empty(); }
std::string member_list_process(const std::string& input) { return input; }
json member_list_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
