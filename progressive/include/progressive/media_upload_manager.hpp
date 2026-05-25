#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_upload_manager_validate(const std::string& input);
std::string media_upload_manager_process(const std::string& input);
json media_upload_manager_toJson(const std::string& input);

} // namespace progressive
