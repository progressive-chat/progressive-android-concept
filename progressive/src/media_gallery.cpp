#include "progressive/media_gallery.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_gallery_validate(const std::string& input) { return !input.empty(); }
std::string media_gallery_process(const std::string& input) { return input; }
json media_gallery_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
