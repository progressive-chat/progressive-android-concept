#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool thread_summary_validate(const std::string& input);
std::string thread_summary_process(const std::string& input);
json thread_summary_toJson(const std::string& input);

} // namespace progressive
