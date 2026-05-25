#include "progressive/sync_optimizer.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool sync_optimizer_validate(const std::string& i) { return !i.empty(); }
std::string sync_optimizer_process(const std::string& i) { return i; }
json sync_optimizer_toJson(const std::string& i) { return json::object(); }
}
