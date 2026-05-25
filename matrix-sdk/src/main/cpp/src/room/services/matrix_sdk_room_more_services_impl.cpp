#include "matrix_sdk/room/services/matrix_sdk_room_services.hpp"
#include "matrix_sdk/matrix_sdk_types.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <chrono>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <unordered_map>

namespace matrix_sdk {

// ============================================================================
// ReadServiceImpl
// ============================================================================
class ReadServiceImpl : public ReadService {
public:
    explicit ReadServiceImpl(const std::string& roomId) : roomId_(roomId), unreadCount_(0), highlightCount_(0), notificationCount_(0) {}

    void markAsRead(const std::string& eventId) override {
        if (eventId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        readMarkerEventId_ = eventId;
        readReceiptEventId_ = eventId;
        lastReadEventId_ = eventId;
        // Reset counters
        unreadCount_ = 0;
        highlightCount_ = 0;
        notificationCount_ = 0;
    }

    void markAllAsRead() override {
        std::lock_guard<std::mutex> lock(mutex_);
        readMarkerEventId_ = lastDisplayedEventId_;
        readReceiptEventId_ = lastDisplayedEventId_;
        lastReadEventId_ = lastDisplayedEventId_;
        unreadCount_ = 0;
        highlightCount_ = 0;
        notificationCount_ = 0;
    }

    void setReadMarker(const std::string& eventId) override {
        if (eventId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        readMarkerEventId_ = eventId;
    }

    void setReadReceipt(const std::string& eventId) override {
        if (eventId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        readReceiptEventId_ = eventId;
    }

    std::string getReadMarkerEventId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return readMarkerEventId_;
    }

    std::string getReadReceiptEventId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return readReceiptEventId_;
    }

    int getUnreadCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return unreadCount_;
    }

    int getHighlightCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return highlightCount_;
    }

    int getNotificationCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return notificationCount_;
    }

    bool hasUnreadMessages() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return unreadCount_ > 0;
    }

    std::string getLastReadEventId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return lastReadEventId_;
    }

    std::string getLastDisplayedEventId() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return lastDisplayedEventId_;
    }

    // ── Internal helpers ──
    void incrementUnread(bool isHighlight, bool isNotification) {
        std::lock_guard<std::mutex> lock(mutex_);
        unreadCount_++;
        if (isHighlight) highlightCount_++;
        if (isNotification) notificationCount_++;
    }

    void setLastDisplayedEventId(const std::string& eventId) {
        std::lock_guard<std::mutex> lock(mutex_);
        lastDisplayedEventId_ = eventId;
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::string readMarkerEventId_;
    std::string readReceiptEventId_;
    std::string lastReadEventId_;
    std::string lastDisplayedEventId_;
    int unreadCount_;
    int highlightCount_;
    int notificationCount_;
};

// ============================================================================
// TypingServiceImpl
// ============================================================================
class TypingServiceImpl : public TypingService {
public:
    explicit TypingServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void sendTyping(bool isTyping, int timeoutMs, SimpleCallback callback) override {
        if (timeoutMs < 0) {
            callback(false, "Timeout must be non-negative");
            return;
        }
        // Clamp timeout to Matrix spec limits (recommended max 30000ms)
        int clampedTimeout = std::min(timeoutMs, 30000);
        if (clampedTimeout == 0 && isTyping) {
            clampedTimeout = 30000; // default timeout
        }

        // In a real impl this would call PUT /rooms/{roomId}/typing/{userId}
        // with body { "typing": isTyping, "timeout": clampedTimeout }
        if (isTyping) {
            startTypingTimer(clampedTimeout);
        } else {
            stopTypingTimer();
        }
        callback(true, "");
    }

    std::vector<std::string> getTypingUsers() const override {
        auto now = std::chrono::steady_clock::now();
        std::vector<std::string> result;
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [userId, expiry] : typingUsers_) {
            if (expiry > now) {
                result.push_back(userId);
            }
        }
        return result;
    }

    int getTypingUsersCount() const override {
        auto now = std::chrono::steady_clock::now();
        int count = 0;
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [userId, expiry] : typingUsers_) {
            if (expiry > now) count++;
        }
        return count;
    }

    bool isUserTyping(const std::string& userId) const override {
        auto now = std::chrono::steady_clock::now();
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = typingUsers_.find(userId);
        if (it != typingUsers_.end()) {
            return it->second > now;
        }
        return false;
    }

    void observeTypingUsers(TypingCallback callback) override {
        if (!callback) return;
        std::lock_guard<std::mutex> lock(observerMutex_);
        observers_.push_back(std::move(callback));
    }

    // ── Internal helpers ──
    void updateTypingUsers(const std::vector<std::string>& users) {
        auto now = std::chrono::steady_clock::now();
        auto expiry = now + std::chrono::milliseconds(30000);
        {
            std::lock_guard<std::mutex> lock(mutex_);
            typingUsers_.clear();
            for (const auto& userId : users) {
                typingUsers_[userId] = expiry;
            }
        }
        notifyObservers(users);
    }

private:
    void startTypingTimer(int timeoutMs) {
        auto expiry = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
        std::lock_guard<std::mutex> lock(mutex_);
        // In a real implementation, this would set a timer for timeoutMs
        // to auto-clear typing status. Here we just set the expiry.
        typingUsers_[roomId_] = expiry;
    }

    void stopTypingTimer() {
        std::lock_guard<std::mutex> lock(mutex_);
        typingUsers_.erase(roomId_);
    }

    void notifyObservers(const std::vector<std::string>& users) {
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto& obs : observers_) {
            obs(users);
        }
    }

    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, std::chrono::steady_clock::time_point> typingUsers_;

    mutable std::mutex observerMutex_;
    std::vector<TypingCallback> observers_;
};

// ============================================================================
// UploadsServiceImpl
// ============================================================================
class UploadsServiceImpl : public UploadsService {
public:
    explicit UploadsServiceImpl(const std::string& baseUrl) : baseUrl_(baseUrl) {}

    void uploadFile(const std::string& localPath, const std::string& filename,
        const std::string& mimeType, ProgressCallback progress, UploadCallback callback) override {
        if (localPath.empty()) {
            if (callback) callback(false, "", "", "Local path is required");
            return;
        }

        // Generate a unique upload ID
        std::string uploadId = generateUploadId();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            UploadRecord record;
            record.uploadId = uploadId;
            record.localPath = localPath;
            record.filename = filename;
            record.mimeType = mimeType;
            record.isActive = true;
            activeUploads_[uploadId] = record;
        }

        // In a real impl this would POST to /_matrix/media/v3/upload
        // with multipart/form-data. The homeserver returns a content_uri (mxc://...)
        // For now, simulate a successful upload with a mock mxc:// URI
        if (progress) {
            progress(50, 100);
        }
        if (progress) {
            progress(100, 100);
        }

        std::string contentUri = "mxc://localhost/" + uploadId;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            activeUploads_.erase(uploadId);
            pendingCount_--;
        }

        if (callback) {
            callback(true, uploadId, contentUri, "");
        }
    }

    void cancelUpload(const std::string& uploadId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = activeUploads_.find(uploadId);
        if (it != activeUploads_.end()) {
            it->second.isActive = false;
            activeUploads_.erase(it);
            pendingCount_--;
        }
    }

    void cancelAllUploads() override {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [id, record] : activeUploads_) {
            record.isActive = false;
        }
        activeUploads_.clear();
        pendingCount_ = 0;
    }

    int getPendingUploadCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return pendingCount_;
    }

    int getActiveUploadCount() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        int count = 0;
        for (const auto& [id, record] : activeUploads_) {
            if (record.isActive) count++;
        }
        return count;
    }

private:
    struct UploadRecord {
        std::string uploadId;
        std::string localPath;
        std::string filename;
        std::string mimeType;
        bool isActive = false;
    };

    std::string generateUploadId() {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        return "upload_" + std::to_string(ms) + "_" + std::to_string(uploadCounter_++);
    }

    std::string baseUrl_;
    mutable std::mutex mutex_;
    std::map<std::string, UploadRecord> activeUploads_;
    int pendingCount_ = 0;
    int uploadCounter_ = 0;
};

// ============================================================================
// DraftServiceImpl
// ============================================================================
class DraftServiceImpl : public DraftService {
public:
    explicit DraftServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    void saveDraft(const std::string& text) override {
        std::lock_guard<std::mutex> lock(mutex_);
        mainDraft_ = text;
    }

    std::string getDraft() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return mainDraft_;
    }

    void deleteDraft() override {
        std::lock_guard<std::mutex> lock(mutex_);
        mainDraft_.clear();
    }

    bool hasDraft() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return !mainDraft_.empty();
    }

    void saveThreadDraft(const std::string& threadId, const std::string& text) override {
        if (threadId.empty()) return;
        std::lock_guard<std::mutex> lock(mutex_);
        threadDrafts_[threadId] = text;
    }

    std::string getThreadDraft(const std::string& threadId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = threadDrafts_.find(threadId);
        if (it != threadDrafts_.end()) {
            return it->second;
        }
        return "";
    }

    void deleteThreadDraft(const std::string& threadId) override {
        std::lock_guard<std::mutex> lock(mutex_);
        threadDrafts_.erase(threadId);
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::string mainDraft_;
    std::map<std::string, std::string> threadDrafts_;
};

// ============================================================================
// RoomPushRuleServiceImpl
// ============================================================================
class RoomPushRuleServiceImpl : public RoomPushRuleService {
public:
    explicit RoomPushRuleServiceImpl(const std::string& roomId)
        : roomId_(roomId), notificationState_(RoomNotificationState::ALL_MESSAGES), pushEnabled_(true) {}

    RoomNotificationState getNotificationState() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return notificationState_;
    }

    void setNotificationState(RoomNotificationState state, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        notificationState_ = state;

        // Push is disabled when muted
        pushEnabled_ = (state != RoomNotificationState::MUTE);

        // In a real impl this would call PUT /pushrules/global/room/{roomId}
        // with appropriate actions based on the state
        callback(true, "");
    }

    bool isPushEnabled() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return pushEnabled_ && notificationState_ != RoomNotificationState::MUTE;
    }

    void setPushEnabled(bool enabled, SimpleCallback callback) override {
        std::lock_guard<std::mutex> lock(mutex_);
        pushEnabled_ = enabled;
        // In a real impl this would enable/disable the push rule for this room
        callback(true, "");
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    RoomNotificationState notificationState_;
    bool pushEnabled_;
};

// ============================================================================
// TagsServiceImpl
// ============================================================================
class TagsServiceImpl : public TagsService {
public:
    explicit TagsServiceImpl(const std::string& roomId) : roomId_(roomId) {}

    std::vector<std::string> getTags() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<std::string> result;
        result.reserve(tags_.size());
        for (const auto& [tag, order] : tags_) {
            result.push_back(tag);
        }
        return result;
    }

    void setTag(const std::string& tag, double order, SimpleCallback callback) override {
        if (tag.empty()) {
            callback(false, "Tag name is required");
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        tags_[tag] = order;
        // In a real impl this would call PUT /user/{userId}/rooms/{roomId}/tags/{tag}
        callback(true, "");
    }

    void removeTag(const std::string& tag, SimpleCallback callback) override {
        if (tag.empty()) {
            callback(false, "Tag name is required");
            return;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        tags_.erase(tag);
        // In a real impl this would call DELETE /user/{userId}/rooms/{roomId}/tags/{tag}
        callback(true, "");
    }

    bool isFavourite() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return tags_.find("m.favourite") != tags_.end();
    }

    bool isLowPriority() const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return tags_.find("m.lowpriority") != tags_.end();
    }

    void setFavourite(bool favourite, SimpleCallback callback) override {
        if (favourite) {
            // m.favourite order is typically 0.0 to put it at the top
            setTag("m.favourite", 0.0, callback);
        } else {
            removeTag("m.favourite", callback);
        }
    }

    void setLowPriority(bool lowPriority, SimpleCallback callback) override {
        if (lowPriority) {
            // m.lowpriority order is typically 1.0 to put it at the bottom
            setTag("m.lowpriority", 1.0, callback);
        } else {
            removeTag("m.lowpriority", callback);
        }
    }

private:
    std::string roomId_;
    mutable std::mutex mutex_;
    std::map<std::string, double> tags_; // tag name -> order
};

// ============================================================================
// Factory functions
// ============================================================================

std::shared_ptr<ReadService> createReadService(const std::string& roomId) {
    return std::make_shared<ReadServiceImpl>(roomId);
}

std::shared_ptr<TypingService> createTypingService(const std::string& roomId) {
    return std::make_shared<TypingServiceImpl>(roomId);
}

std::shared_ptr<UploadsService> createUploadsService(const std::string& baseUrl) {
    return std::make_shared<UploadsServiceImpl>(baseUrl);
}

std::shared_ptr<DraftService> createDraftService(const std::string& roomId) {
    return std::make_shared<DraftServiceImpl>(roomId);
}

std::shared_ptr<RoomPushRuleService> createRoomPushRuleService(const std::string& roomId) {
    return std::make_shared<RoomPushRuleServiceImpl>(roomId);
}

std::shared_ptr<TagsService> createTagsService(const std::string& roomId) {
    return std::make_shared<TagsServiceImpl>(roomId);
}

} // namespace matrix_sdk
