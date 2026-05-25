#include "progressive/olm_account_manager.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool olm_account_manager_validate(const std::string& i) { return !i.empty(); }
std::string olm_account_manager_process(const std::string& i) { return i; }
json olm_account_manager_toJson(const std::string& i) { return json::object(); }
}
