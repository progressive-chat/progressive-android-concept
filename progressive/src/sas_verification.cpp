#include "progressive/sas_verification.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool sas_verification_validate(const std::string& i) { return !i.empty(); }
std::string sas_verification_process(const std::string& i) { return i; }
json sas_verification_toJson(const std::string& i) { return json::object(); }
}
