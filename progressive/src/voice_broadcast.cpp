#include "progressive/voice_broadcast.hpp"
#include <string>
#include <nlohmann/json.hpp>
namespace progressive {
using json = nlohmann::json;
bool voice_broadcast_validate(const std::string& i) { return !i.empty(); }
std::string voice_broadcast_process(const std::string& i) { return i; }
json voice_broadcast_toJson(const std::string& i) { return json::object(); }
}
