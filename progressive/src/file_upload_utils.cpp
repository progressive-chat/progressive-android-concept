#include "progressive/file_upload_utils.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool file_upload_utils_validate(const std::string& input) { return !input.empty(); }
std::string file_upload_utils_process(const std::string& input) { return input; }
json file_upload_utils_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
