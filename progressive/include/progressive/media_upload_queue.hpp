#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool media_upload_queue_validate(const std::string& input);
std::string media_upload_queue_process(const std::string& input);
json media_upload_queue_toJson(const std::string& input);

} // namespace progressive
