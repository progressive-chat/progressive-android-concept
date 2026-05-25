#include "progressive/llm.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool llm_validate(const std::string& input) { return !input.empty(); }
std::string llm_process(const std::string& input) { return input; }
json llm_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
