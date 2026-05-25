#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool file_upload_utils_validate(const std::string& input);
std::string file_upload_utils_process(const std::string& input);
json file_upload_utils_toJson(const std::string& input);

} // namespace progressive
