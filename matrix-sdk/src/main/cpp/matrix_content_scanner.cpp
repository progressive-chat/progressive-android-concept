#include "matrix_content_scanner.hpp"
namespace matrix_sdk {
ContentScanner::ContentScanner() = default;
ContentScanner::~ContentScanner() = default;
ScanResult ContentScanner::scanContent(const std::string& mxcUri) { return {true, ""}; }
ScanResult ContentScanner::scanUrl(const std::string& url) { return {true, ""}; }
} // namespace matrix_sdk
