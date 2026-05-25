#pragma once
#include <string>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;
struct ScanResult { bool clean = true; std::string info; };
class ContentScanner {
public:
    ContentScanner(); ~ContentScanner();
    ScanResult scanContent(const std::string& mxcUri);
    ScanResult scanUrl(const std::string& url);
    bool isEnabled() const { return m_enabled; }
    void setEnabled(bool e) { m_enabled = e; }
private:
    bool m_enabled = false;
    std::string m_scannerUrl;
};
} // namespace matrix_sdk
