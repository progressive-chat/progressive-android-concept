#include "progressive/media_upload_queue.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_upload_queue_validate(const std::string& input) { return !input.empty(); }
std::string media_upload_queue_process(const std::string& input) { return input; }
json media_upload_queue_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
