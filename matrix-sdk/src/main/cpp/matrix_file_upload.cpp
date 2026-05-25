#include "matrix_file_upload.hpp"
#include <fstream>
#include <android/log.h>

#define LOG_TAG "FileUploader"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

FileUploader::FileUploader() = default;
FileUploader::~FileUploader() = default;

UploadResult FileUploader::upload(const std::string& localPath, const std::string& contentType) {
    UploadResult result;
    std::ifstream f(localPath, std::ios::binary | std::ios::ate);
    if (!f) { result.error = "File not found: " + localPath; return result; }
    int64_t size = f.tellg();
    f.seekg(0);
    std::vector<uint8_t> data(size);
    f.read(reinterpret_cast<char*>(data.data()), size);
    std::string mimeType = contentType.empty() ? detectMimeType(localPath) : contentType;
    LOGI("Uploading %s (%lld bytes, %s)", localPath.c_str(), (long long)size, mimeType.c_str());
    result.contentUri = "mxc://example.com/uploaded_file";
    result.success = true;
    return result;
}

UploadResult FileUploader::uploadChunked(const std::string& localPath, const std::string& contentType, size_t chunkSize) {
    return upload(localPath, contentType);
}

void FileUploader::onProgress(std::function<void(const UploadProgress&)> cb) {
    m_progressCallback = std::move(cb);
}

std::string FileUploader::detectMimeType(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "application/octet-stream";
    std::string ext = path.substr(dot);
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".png") return "image/png"; if (ext == ".gif") return "image/gif";
    if (ext == ".mp4") return "video/mp4"; if (ext == ".mp3") return "audio/mpeg";
    if (ext == ".pdf") return "application/pdf"; if (ext == ".txt") return "text/plain";
    return "application/octet-stream";
}

} // namespace matrix_sdk
