#include "progressive/media_upload_manager.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_upload_manager_validate(const std::string& input) { return !input.empty(); }
std::string media_upload_manager_process(const std::string& input) { return input; }
json media_upload_manager_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
