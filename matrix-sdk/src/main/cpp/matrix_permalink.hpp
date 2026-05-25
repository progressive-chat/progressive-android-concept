#pragma once
#include <string>
#include <vector>

namespace matrix_sdk {

struct PermalinkInfo {
    std::string type; // "user", "room", "event", "group"
    std::string identifier; std::string viaServer;
    bool valid = false;
};

class PermalinkParser {
public:
    static PermalinkInfo parse(const std::string& url);
    static std::string createUserLink(const std::string& userId);
    static std::string createRoomLink(const std::string& roomId, const std::vector<std::string>& viaServers = {});
    static std::string createEventLink(const std::string& roomId, const std::string& eventId, const std::vector<std::string>& viaServers = {});
    static bool isMatrixToUrl(const std::string& url);
    static std::string matrixToUrlToRoomId(const std::string& url);
};
} // namespace matrix_sdk
