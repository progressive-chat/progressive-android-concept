#include "progressive/config_loader.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool config_loader_validate(const std::string& i) { return !i.empty(); }
std::string config_loader_process(const std::string& i) { return i; }
json config_loader_toJson(const std::string& i) { return json::object(); }
}
