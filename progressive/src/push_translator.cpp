#include "progressive/push_translator.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool push_translator_validate(const std::string& input) { return !input.empty(); }
std::string push_translator_process(const std::string& input) { return input; }
json push_translator_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
