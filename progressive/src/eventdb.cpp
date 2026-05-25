#include "progressive/eventdb.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool eventdb_validate(const std::string& i) { return !i.empty(); }
std::string eventdb_process(const std::string& i) { return i; }
json eventdb_toJson(const std::string& i) { return json::object(); }
}
