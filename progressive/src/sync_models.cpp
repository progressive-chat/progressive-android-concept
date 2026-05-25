#include "progressive/sync_models.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool sync_models_validate(const std::string& i) { return !i.empty(); }
std::string sync_models_process(const std::string& i) { return i; }
json sync_models_toJson(const std::string& i) { return json::object(); }
}
