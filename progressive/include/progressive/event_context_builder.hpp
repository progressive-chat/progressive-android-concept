#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool event_context_builder_validate(const std::string& input);
std::string event_context_builder_process(const std::string& input);
json event_context_builder_toJson(const std::string& input);

} // namespace progressive
