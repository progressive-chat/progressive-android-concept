#include "progressive/media_compressor.hpp"
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_compressor_validate(const std::string& input) { return !input.empty(); }
std::string media_compressor_process(const std::string& input) { return input; }
json media_compressor_toJson(const std::string& input) { return json::object(); }

} // namespace progressive
