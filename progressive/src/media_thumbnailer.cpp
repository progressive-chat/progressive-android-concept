#include "progressive/media_thumbnailer.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_thumbnailer_validate(const std::string& input) { return !input.empty(); }
std::string media_thumbnailer_process(const std::string& input) { return input; }
json media_thumbnailer_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
