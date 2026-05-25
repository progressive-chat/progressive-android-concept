#include "progressive/content_builder_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool content_builder_utils_validate(const std::string& input) { return !input.empty(); }
std::string content_builder_utils_process(const std::string& input) { return input; }
json content_builder_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
