#pragma once
#include <string>
#include <vector>
#include <map>
#include <nlohmann/json.hpp>

namespace matrix_sdk {
using json = nlohmann::json;

struct SyncParseContext;
bool parseSyncResponse(const std::string& rawJson, SyncParseContext& ctx);
void parseRoomData(const std::string& roomId, const json& roomData, const std::string& membership, SyncParseContext& ctx);
void processTimelineEvent(const std::string& roomId, const json& event);
void processStateEvent(const std::string& roomId, const json& event);
void processTypingEvent(const std::string& roomId, const json& event);
void processReceiptEvent(const std::string& roomId, const json& event);
void updateRoomUnreadCounts(const std::string& roomId, int notificationCount, int highlightCount);
void updateRoomSummary(const std::string& roomId, int joinedMembers, int invitedMembers);
void recordMembershipChange(const std::string& roomId, const std::string& userId, const std::string& membership);
bool processAccountDataSync(const json& accountData);
void processDeviceLists(const std::vector<std::string>& changed, const std::vector<std::string>& left);
void checkOneTimeKeyCount(int count, int threshold);
enum class SyncErrorAction { RETRY, RESET_CACHE, RE_LOGIN, CONTINUE };
SyncErrorAction determineSyncAction(const std::string& error, int httpStatus);
std::string buildSyncFilter(int timelineLimit, bool includePresence, bool includeTyping);
int calculateBackoffMs(int consecutiveErrors, int baseDelayMs);
struct SyncStats { int64_t totalSyncs, successfulSyncs, failedSyncs, totalEvents, totalStateEvents, totalLatencyMs, lastSyncMs; int consecutiveErrors; };
SyncStats getSyncStats();
void resetSyncStats();
} // namespace matrix_sdk
