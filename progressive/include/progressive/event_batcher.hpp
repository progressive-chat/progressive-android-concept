#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_batcher_validate(const std::string& input);
std::string event_batcher_process(const std::string& input);
json event_batcher_toJson(const std::string& input);

} // namespace progressive
