#include "progressive/device_trust.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool device_trust_validate(const std::string& i) { return !i.empty(); }
std::string device_trust_process(const std::string& i) { return i; }
json device_trust_toJson(const std::string& i) { return json::object(); }
}
