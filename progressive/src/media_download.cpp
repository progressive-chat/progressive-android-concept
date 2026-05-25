#include "progressive/media_download.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_download_validate(const std::string& input) { return !input.empty(); }
std::string media_download_process(const std::string& input) { return input; }
json media_download_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
