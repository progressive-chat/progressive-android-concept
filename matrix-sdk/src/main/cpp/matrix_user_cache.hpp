#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <mutex>
#include <functional>
#include <nlohmann/json.hpp>

namespace matrix_sdk {

using json = nlohmann::json;

// ─────────────────────────────────────────────────────────────
//  In-memory user profile cache with TTL, batch fetch,
//  presence tracking, and avatar resolution
// ─────────────────────────────────────────────────────────────

struct CachedUserProfile {
    std::string userId;
    std::string displayName;
    std::string avatarUrl;
    std::string presence;          // online, offline, unavailable
    std::string statusMessage;
    int64_t lastActiveAgo = 0;     // ms
    bool currentlyActive = false;
    bool isIgnored = false;
    json deviceKeys;               // cached device identity keys
    std::vector<std::string> devices;
    int64_t fetchedAt = 0;
    int64_t expiresAt = 0;
    int fetchCount = 0;
    bool stale = true;
};

struct UserCacheConfig {
    size_t maxCachedUsers = 5000;
    int64_t profileTtlMs = 3600000;       // 1 hour
    int64_t presenceTtlMs = 300000;       // 5 minutes
    int64_t avatarTtlMs = 86400000;       // 24 hours
    int64_t deviceKeysTtlMs = 3600000;    // 1 hour
    int batchFetchSize = 50;
    bool autoEvictStale = true;
    int evictionIntervalMs = 300000;       // 5 minutes
    std::string defaultAvatarUrl;
};

enum class UserProfileField {
    DISPLAY_NAME,
    AVATAR_URL,
    PRESENCE,
    STATUS_MESSAGE,
    DEVICE_KEYS,
    ALL
};

using ProfileUpdateCallback = std::function<void(const std::string& userId,
                                                    UserProfileField field)>;
using ProfileFetchCallback = std::function<void(const std::string& userId,
                                                  const CachedUserProfile& profile)>;
using BatchProfileCallback = std::function<void(
    const std::vector<CachedUserProfile>& profiles)>;

class MatrixUserCache {
public:
    MatrixUserCache();
    explicit MatrixUserCache(const UserCacheConfig& config);
    ~MatrixUserCache();

    void configure(const UserCacheConfig& config);

    // ── Profile CRUD ──
    bool putProfile(const CachedUserProfile& profile);
    std::optional<CachedUserProfile> getProfile(const std::string& userId);
    bool hasProfile(const std::string& userId) const;
    bool removeProfile(const std::string& userId);
    void clear();

    // ── Display name ──
    bool setDisplayName(const std::string& userId, const std::string& name);
    std::optional<std::string> getDisplayName(const std::string& userId);
    std::string getDisplayNameOrId(const std::string& userId); // fallback to userId

    // ── Avatar ──
    bool setAvatarUrl(const std::string& userId, const std::string& url);
    std::optional<std::string> getAvatarUrl(const std::string& userId);
    std::string resolveAvatarUrl(const std::string& userId,
                                   const std::string& homeserverUrl);

    // ── Presence ──
    bool setPresence(const std::string& userId, const std::string& presence,
                      const std::string& statusMsg = "",
                      int64_t lastActiveAgo = 0,
                      bool currentlyActive = false);
    std::optional<std::string> getPresence(const std::string& userId);
    std::vector<std::string> getOnlineUsers();
    bool isUserOnline(const std::string& userId);

    // ── Device keys ──
    bool setDeviceKeys(const std::string& userId, const json& keys);
    std::optional<json> getDeviceKeys(const std::string& userId);
    bool setUserDevices(const std::string& userId,
                         const std::vector<std::string>& deviceIds);
    std::vector<std::string> getUserDevices(const std::string& userId);

    // ── Ignore list ──
    bool ignoreUser(const std::string& userId);
    bool unignoreUser(const std::string& userId);
    bool isUserIgnored(const std::string& userId);
    std::vector<std::string> getIgnoredUsers();

    // ── Batch operations ──
    std::vector<CachedUserProfile> getProfiles(
        const std::vector<std::string>& userIds);
    bool putProfiles(const std::vector<CachedUserProfile>& profiles);

    // ── Search ──
    std::vector<std::string> searchUsers(const std::string& query, int limit = 20);
    std::vector<std::string> searchByDisplayName(const std::string& query,
                                                    int limit = 20);

    // ── Caching ──
    void evictStale();
    void evictLRU(size_t targetSize);
    size_t size() const;
    size_t capacity() const { return m_config.maxCachedUsers; }
    bool isStale(const std::string& userId) const;
    void markStale(const std::string& userId);
    void refreshExpiry(const std::string& userId, int64_t ttlMs = 0);

    // ── Callbacks ──
    void setOnProfileUpdate(ProfileUpdateCallback cb) {
        m_profileUpdateCallback = std::move(cb);
    }

    // ── Cache statistics ──
    struct CacheStats {
        size_t totalUsers = 0;
        size_t staleUsers = 0;
        size_t onlineUsers = 0;
        size_t ignoredUsers = 0;
        int64_t memoryEstimate = 0;
    };
    CacheStats getStats() const;

    // ── Persistence ──
    using PersistFunc = std::function<void(const std::string& userId,
                                            const CachedUserProfile&)>;
    void setPersistCallback(PersistFunc cb) { m_persistCallback = std::move(cb); }

private:
    CachedUserProfile* getOrCreate(const std::string& userId);
    void touchProfile(const std::string& userId);
    int64_t nowMs() const;
    void checkAutoEviction();
    std::string computeProfileKey(const std::string& userId) const;

    UserCacheConfig m_config;
    mutable std::mutex m_mutex;

    std::map<std::string, CachedUserProfile> m_profiles;
    std::vector<std::string> m_ignoredUsers;

    ProfileUpdateCallback m_profileUpdateCallback;
    PersistFunc m_persistCallback;

    int64_t m_lastEvictionMs = 0;
};

// ── Display name disambiguation ──

std::string disambiguateDisplayName(const std::string& displayName,
                                      const std::string& userId,
                                      const std::vector<std::string>& otherNames);

} // namespace matrix_sdk
