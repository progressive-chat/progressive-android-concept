#include "progressive/media_utils.hpp"
#include "progressive/media_compressor.hpp"
#include "progressive/media_thumbnailer.hpp"
#include "progressive/media_download.hpp"
#include <android/log.h>
#include <cmath>
#include <fstream>
#include <vector>

#define LOG_TAG "MediaHandler"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace progressive {

// ==== Image Processing ====

struct ImageInfo {
    int width = 0; int height = 0; std::string mimeType;
    int orientation = 1; int64_t fileSize = 0; bool animated = false;
};

class ImageProcessor {
public:
    static ImageInfo getInfo(const std::string& path) {
        ImageInfo info;
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (f) { info.fileSize = f.tellg(); }
        return info;
    }

    static bool resize(const std::string& input, const std::string& output,
                       int targetW, int targetH, int quality = 85) {
        LOGI("Resizing image: %s -> %s (%dx%d, q=%d)", input.c_str(), output.c_str(), targetW, targetH, quality);
        return true;
    }

    static bool crop(const std::string& input, const std::string& output,
                     int x, int y, int w, int h) {
        LOGI("Cropping image: %s (%d,%d %dx%d)", input.c_str(), x, y, w, h);
        return true;
    }

    static bool rotate(const std::string& input, const std::string& output, int degrees) {
        LOGI("Rotating image: %s by %d degrees", input.c_str(), degrees);
        return true;
    }

    static bool generateThumbnail(const std::string& input, const std::string& output,
                                   int maxW, int maxH) {
        ImageInfo info = getInfo(input);
        if (info.width <= maxW && info.height <= maxH) {
            return true; // No resizing needed
        }
        double ratio = std::min((double)maxW / info.width, (double)maxH / info.height);
        int newW = (int)(info.width * ratio);
        int newH = (int)(info.height * ratio);
        return resize(input, output, newW, newH, 80);
    }
};

// ==== Video Processing ====

struct VideoInfo {
    int width = 0; int height = 0; int durationMs = 0;
    int bitrate = 0; std::string codec; int64_t fileSize = 0;
    float fps = 0.0f; bool hasAudio = false;
};

class VideoProcessor {
public:
    static VideoInfo getInfo(const std::string& path) {
        VideoInfo info;
        LOGI("Analyzing video: %s", path.c_str());
        return info;
    }

    static bool compress(const std::string& input, const std::string& output,
                         int maxBitrate, int maxResolution) {
        LOGI("Compressing video: %s, bitrate=%d, res=%d", input.c_str(), maxBitrate, maxResolution);
        return true;
    }

    static bool extractThumbnail(const std::string& input, const std::string& output,
                                  int timeMs) {
        LOGI("Extracting thumbnail from %s at %dms", input.c_str(), timeMs);
        return true;
    }

    static bool transcode(const std::string& input, const std::string& output,
                          const std::string& format) {
        LOGI("Transcoding %s to %s format", input.c_str(), format.c_str());
        return true;
    }
};

// ==== Audio Processing ====

struct AudioInfo {
    int durationMs = 0; int sampleRate = 0; int channels = 0;
    int bitrate = 0; std::string codec; int64_t fileSize = 0;
};

class AudioProcessor {
public:
    static AudioInfo getInfo(const std::string& path) {
        AudioInfo info;
        LOGI("Analyzing audio: %s", path.c_str());
        return info;
    }

    static std::vector<int16_t> decode(const std::string& path) {
        std::vector<int16_t> samples;
        LOGI("Decoding audio: %s", path.c_str());
        return samples;
    }

    static bool encode(const std::vector<int16_t>& samples, const std::string& output,
                       int sampleRate, int channels) {
        LOGI("Encoding audio: %s, rate=%d, ch=%d", output.c_str(), sampleRate, channels);
        return true;
    }

    static bool transcode(const std::string& input, const std::string& output,
                          const std::string& format, int targetBitrate) {
        LOGI("Transcoding audio: %s -> %s (%s, %d)", input.c_str(), output.c_str(),
             format.c_str(), targetBitrate);
        return true;
    }
};

// ==== Media Cache Manager ====

class MediaCacheManager {
public:
    MediaCacheManager(const std::string& cacheDir) : m_cacheDir(cacheDir) {}
    ~MediaCacheManager() = default;

    std::string get(const std::string& uri) {
        auto it = m_cache.find(uri);
        if (it != m_cache.end()) return it->second;
        return "";
    }

    void put(const std::string& uri, const std::string& localPath) {
        m_cache[uri] = localPath;
        m_lru.push_back(uri);
        if (m_lru.size() > m_maxEntries) {
            m_cache.erase(m_lru.front());
            m_lru.erase(m_lru.begin());
        }
    }

    bool has(const std::string& uri) const {
        return m_cache.find(uri) != m_cache.end();
    }

    void clear() { m_cache.clear(); m_lru.clear(); }
    size_t size() const { return m_cache.size(); }
    void setMaxEntries(size_t max) { m_maxEntries = max; }

    size_t totalSizeBytes() const {
        size_t total = 0;
        for (auto& [uri, path] : m_cache) {
            std::ifstream f(path, std::ios::binary | std::ios::ate);
            if (f) total += f.tellg();
        }
        return total;
    }

private:
    std::string m_cacheDir;
    std::map<std::string, std::string> m_cache;
    std::vector<std::string> m_lru;
    size_t m_maxEntries = 500;
};

// ==== File Type Detector ====

class FileTypeDetector {
public:
    static std::string detectByMagic(const std::vector<uint8_t>& data) {
        if (data.size() < 4) return "application/octet-stream";
        // JPEG: FF D8 FF
        if (data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) return "image/jpeg";
        // PNG: 89 50 4E 47
        if (data[0] == 0x89 && data[1] == 0x50 && data[2] == 0x4E && data[3] == 0x47) return "image/png";
        // GIF: 47 49 46
        if (data[0] == 0x47 && data[1] == 0x49 && data[2] == 0x46) return "image/gif";
        // WebP: 52 49 46 46 ... 57 45 42 50
        if (data.size() >= 12 && data[0]=='R' && data[1]=='I' && data[2]=='F' && data[3]=='F' &&
            data[8]=='W' && data[9]=='E' && data[10]=='B' && data[11]=='P') return "image/webp";
        // MP4: ... 66 74 79 70
        if (data.size() >= 8 && data[4]=='f' && data[5]=='t' && data[6]=='y' && data[7]=='p') return "video/mp4";
        // PDF: 25 50 44 46
        if (data[0] == 0x25 && data[1] == 0x50 && data[2] == 0x44 && data[3] == 0x46) return "application/pdf";
        // ZIP: 50 4B
        if (data[0] == 0x50 && data[1] == 0x4B) return "application/zip";
        // MP3: FF FB or ID3
        if (data[0] == 0xFF && (data[1] & 0xFE) == 0xFA) return "audio/mpeg";
        if (data.size() >= 3 && data[0]=='I' && data[1]=='D' && data[2]=='3') return "audio/mpeg";
        // OGG: 4F 67 67 53
        if (data[0]=='O' && data[1]=='g' && data[2]=='g' && data[3]=='S') return "audio/ogg";
        return "application/octet-stream";
    }

    static std::string detectByExtension(const std::string& path) {
        size_t dot = path.rfind('.');
        if (dot == std::string::npos) return "application/octet-stream";
        std::string ext = path.substr(dot);
        static std::map<std::string, std::string> extMap = {
            {".jpg","image/jpeg"},{".jpeg","image/jpeg"},{".png","image/png"},
            {".gif","image/gif"},{".webp","image/webp"},{".bmp","image/bmp"},
            {".svg","image/svg+xml"},{".mp4","video/mp4"},{".webm","video/webm"},
            {".mkv","video/x-matroska"},{".mov","video/quicktime"},{".avi","video/x-msvideo"},
            {".mp3","audio/mpeg"},{".ogg","audio/ogg"},{".wav","audio/wav"},
            {".flac","audio/flac"},{".aac","audio/aac"},{".m4a","audio/mp4"},
            {".pdf","application/pdf"},{".txt","text/plain"},{".json","application/json"},
            {".zip","application/zip"},{".apk","application/vnd.android.package-archive"},
        };
        auto it = extMap.find(ext);
        return it != extMap.end() ? it->second : "application/octet-stream";
    }
};

// ==== Upload Progress Tracker ====

class UploadProgressTracker {
public:
    struct UploadTask {
        std::string id; std::string path; std::string mxcUri;
        int64_t totalBytes = 0; int64_t uploadedBytes = 0;
        float progress = 0.0f; bool complete = false; bool failed = false;
        std::string error;
    };

    UploadTask* createTask(const std::string& path) {
        std::string id = "upload_" + std::to_string(m_nextId++);
        UploadTask task;
        task.id = id; task.path = path;
        std::ifstream f(path, std::ios::binary | std::ios::ate);
        if (f) task.totalBytes = f.tellg();
        m_tasks[id] = task;
        return &m_tasks[id];
    }

    void updateProgress(const std::string& id, int64_t uploadedBytes) {
        auto it = m_tasks.find(id);
        if (it != m_tasks.end()) {
            it->second.uploadedBytes = uploadedBytes;
            if (it->second.totalBytes > 0) {
                it->second.progress = (float)uploadedBytes / it->second.totalBytes;
            }
        }
    }

    void markComplete(const std::string& id, const std::string& mxcUri) {
        auto it = m_tasks.find(id);
        if (it != m_tasks.end()) {
            it->second.complete = true;
            it->second.progress = 1.0f;
            it->second.mxcUri = mxcUri;
        }
    }

    void markFailed(const std::string& id, const std::string& error) {
        auto it = m_tasks.find(id);
        if (it != m_tasks.end()) {
            it->second.failed = true;
            it->second.error = error;
        }
    }

    UploadTask* getTask(const std::string& id) {
        auto it = m_tasks.find(id);
        return it != m_tasks.end() ? &it->second : nullptr;
    }

    std::vector<UploadTask> getActiveTasks() const {
        std::vector<UploadTask> active;
        for (auto& [id, task] : m_tasks) {
            if (!task.complete && !task.failed) active.push_back(task);
        }
        return active;
    }

    void clearCompleted() {
        auto it = m_tasks.begin();
        while (it != m_tasks.end()) {
            if (it->second.complete || it->second.failed) {
                it = m_tasks.erase(it);
            } else { ++it; }
        }
    }

private:
    std::map<std::string, UploadTask> m_tasks;
    uint64_t m_nextId = 1;
};

// ==== Download Manager ====

class DownloadManager {
public:
    struct DownloadTask {
        std::string id; std::string url; std::string destPath;
        int64_t totalBytes = 0; int64_t downloadedBytes = 0;
        float progress = 0.0f; bool complete = false; bool failed = false;
        std::string error; int retryCount = 0;
    };

    DownloadTask* enqueue(const std::string& url, const std::string& destPath) {
        std::string id = "dl_" + std::to_string(m_nextId++);
        DownloadTask task;
        task.id = id; task.url = url; task.destPath = destPath;
        m_tasks[id] = task;
        return &m_tasks[id];
    }

    void cancel(const std::string& id) { m_tasks.erase(id); }
    void cancelAll() { m_tasks.clear(); }
    int activeCount() const { return (int)m_tasks.size(); }

private:
    std::map<std::string, DownloadTask> m_tasks;
    uint64_t m_nextId = 1;
};

// ==== Media Gallery Manager ====

class MediaGalleryManager {
public:
    struct GalleryItem {
        std::string uri; std::string mimeType; int64_t dateTaken = 0;
        int width = 0, height = 0; int64_t size = 0; std::string displayName;
        bool isVideo = false; int duration = 0;
    };

    void scanDirectory(const std::string& path) {
        LOGI("Scanning gallery directory: %s", path.c_str());
        m_items.clear();
    }

    std::vector<GalleryItem> getAll() const { return m_items; }
    std::vector<GalleryItem> getImages() const {
        std::vector<GalleryItem> result;
        for (auto& item : m_items) if (!item.isVideo) result.push_back(item);
        return result;
    }
    std::vector<GalleryItem> getVideos() const {
        std::vector<GalleryItem> result;
        for (auto& item : m_items) if (item.isVideo) result.push_back(item);
        return result;
    }

    void sortByDate(bool ascending = false) {
        std::sort(m_items.begin(), m_items.end(),
            [ascending](auto& a, auto& b) {
                return ascending ? a.dateTaken < b.dateTaken : a.dateTaken > b.dateTaken;
            });
    }

private:
    std::vector<GalleryItem> m_items;
};

} // namespace progressive
