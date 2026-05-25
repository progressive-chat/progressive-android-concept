#include "progressive/link_previewer.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool link_previewer_validate(const std::string& input) { return !input.empty(); }
std::string link_previewer_process(const std::string& input) { return input; }
json link_previewer_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
