#include "progressive/encrypted_file.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool encrypted_file_validate(const std::string& input) { return !input.empty(); }
std::string encrypted_file_process(const std::string& input) { return input; }
json encrypted_file_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
