#include "progressive/avatar_cropper.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool avatar_cropper_validate(const std::string& input) { return !input.empty(); }
std::string avatar_cropper_process(const std::string& input) { return input; }
json avatar_cropper_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
