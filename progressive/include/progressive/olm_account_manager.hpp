#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace progressive {

using json = nlohmann::json;

bool olm_account_manager_validate(const std::string& input);
std::string olm_account_manager_process(const std::string& input);
json olm_account_manager_toJson(const std::string& input);

} // namespace progressive
