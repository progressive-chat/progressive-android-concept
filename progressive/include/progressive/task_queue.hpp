#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool task_queue_validate(const std::string& input);
std::string task_queue_process(const std::string& input);
json task_queue_toJson(const std::string& input);

} // namespace progressive
