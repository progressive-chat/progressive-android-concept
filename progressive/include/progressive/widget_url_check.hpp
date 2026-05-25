#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool widget_url_check_validate(const std::string& input);
std::string widget_url_check_process(const std::string& input);
json widget_url_check_toJson(const std::string& input);

} // namespace progressive
