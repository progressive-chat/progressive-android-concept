#include "matrix_url_preview.hpp"
#include <regex>
#include <sstream>

namespace matrix_sdk {

UrlPreviewer::UrlPreviewer() = default;
UrlPreviewer::~UrlPreviewer() = default;

UrlPreview UrlPreviewer::fetchPreview(const std::string& url, int64_t ts) {
    UrlPreview p;
    p.url = url;
    return p;
}

std::string UrlPreviewer::toFormattedBody(const UrlPreview& preview,
                                            const std::string& originalUrl) {
    std::ostringstream os;
    os << "<blockquote>";
    if (!preview.imageUrl.empty()) {
        os << "<img src=\"" << preview.imageUrl << "\" alt=\"preview\"/>";
    }
    os << "<a href=\"" << originalUrl << "\">"
       << preview.title << "</a><br/>"
       << preview.description
       << "</blockquote>";
    return os.str();
}

bool UrlPreviewer::isUrl(const std::string& text) const {
    std::regex urlRe("https?://[^ ]+");
    return std::regex_match(text, urlRe);
}

std::vector<std::string> UrlPreviewer::extractUrls(const std::string& text) const {
    std::vector<std::string> urls;
    std::regex urlRe("https?://[^ ]+");
    auto begin = std::sregex_iterator(text.begin(), text.end(), urlRe);
    auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        urls.push_back(it->str());
    }
    return urls;
}

std::string UrlPreviewer::fetchOgData(const std::string& html) { return ""; }
std::string UrlPreviewer::extractMetaTag(const std::string& html,
                                            const std::string& property) const { return ""; }
std::string UrlPreviewer::extractTitle(const std::string& html) const { return ""; }

} // namespace matrix_sdk
