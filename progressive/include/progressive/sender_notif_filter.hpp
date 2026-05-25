#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool sender_notif_filter_validate(const std::string& input);
std::string sender_notif_filter_process(const std::string& input);
json sender_notif_filter_toJson(const std::string& input);

} // namespace progressive
