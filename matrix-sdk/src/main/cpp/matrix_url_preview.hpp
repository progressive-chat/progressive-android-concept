#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct UrlPreview {
    std::string url; std::string title; std::string description;
    std::string imageUrl; std::string siteName;
    int imageWidth = 0, imageHeight = 0; int64_t imageSize = 0;
    bool valid = false;
};

class UrlPreviewer {
public:
    UrlPreviewer(); ~UrlPreviewer();
    UrlPreview fetchPreview(const std::string& url, int64_t ts = 0);
    std::string toFormattedBody(const UrlPreview& preview, const std::string& originalUrl);
    bool isUrl(const std::string& text) const;
    std::vector<std::string> extractUrls(const std::string& text) const;
private:
    std::string fetchOgData(const std::string& html);
    std::string extractMetaTag(const std::string& html, const std::string& property) const;
    std::string extractTitle(const std::string& html) const;
};
} // namespace matrix_sdk
