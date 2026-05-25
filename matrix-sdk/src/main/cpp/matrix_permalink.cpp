#include "matrix_permalink.hpp"
#include <regex>
#include <sstream>

namespace matrix_sdk {

PermalinkInfo PermalinkParser::parse(const std::string& url) {
    PermalinkInfo info;
    std::regex re(R"(matrix\.to/#/(@[^?]+|![^?]+|\$[^?]+))");
    std::smatch match;
    if (std::regex_search(url, match, re)) {
        info.identifier = match[1];
        info.valid = true;
        if (info.identifier[0] == '@') info.type = "user";
        else if (info.identifier[0] == '!') info.type = "room";
        else if (info.identifier[0] == '$') info.type = "event";
    }
    return info;
}

std::string PermalinkParser::createUserLink(const std::string& userId) {
    return "https://matrix.to/#/" + userId;
}

std::string PermalinkParser::createRoomLink(const std::string& roomId, const std::vector<std::string>& viaServers) {
    std::ostringstream os;
    os << "https://matrix.to/#/" << roomId;
    for (size_t i = 0; i < viaServers.size(); i++) {
        os << (i == 0 ? "?via=" : "&via=") << viaServers[i];
    }
    return os.str();
}

std::string PermalinkParser::createEventLink(const std::string& roomId, const std::string& eventId, const std::vector<std::string>& viaServers) {
    std::ostringstream os;
    os << "https://matrix.to/#/" << roomId << "/" << eventId;
    for (size_t i = 0; i < viaServers.size(); i++) {
        os << (i == 0 ? "?via=" : "&via=") << viaServers[i];
    }
    return os.str();
}

bool PermalinkParser::isMatrixToUrl(const std::string& url) {
    return url.find("matrix.to") != std::string::npos;
}

std::string PermalinkParser::matrixToUrlToRoomId(const std::string& url) {
    auto info = parse(url);
    return info.type == "room" ? info.identifier : "";
}

} // namespace matrix_sdk
