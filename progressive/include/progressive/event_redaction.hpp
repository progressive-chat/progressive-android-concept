#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_redaction_validate(const std::string& input);
std::string event_redaction_process(const std::string& input);
json event_redaction_toJson(const std::string& input);

} // namespace progressive
