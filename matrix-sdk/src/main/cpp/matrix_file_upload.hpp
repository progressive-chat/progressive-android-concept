#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct UploadProgress {
    int64_t bytesUploaded = 0; int64_t totalBytes = 0;
    float progress = 0.0f; bool complete = false; std::string error;
};

struct UploadResult {
    std::string contentUri; bool success = false; std::string error;
};

class FileUploader {
public:
    FileUploader(); ~FileUploader();
    UploadResult upload(const std::string& localPath, const std::string& contentType = "");
    UploadResult uploadChunked(const std::string& localPath, const std::string& contentType = "", size_t chunkSize = 1*1024*1024);
    void setAccessToken(const std::string& token) { m_accessToken = token; }
    void setHomeserver(const std::string& url) { m_homeserver = url; }
    void onProgress(std::function<void(const UploadProgress&)> cb);
private:
    std::string m_accessToken; std::string m_homeserver;
    std::function<void(const UploadProgress&)> m_progressCallback;
    std::string detectMimeType(const std::string& path);
};
} // namespace matrix_sdk
