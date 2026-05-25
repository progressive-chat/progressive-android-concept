#include "matrix_room_manager.hpp"
#include <algorithm>
#include <android/log.h>

#define LOG_TAG "RoomManager"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

namespace matrix_sdk {

MatrixRoomManager::MatrixRoomManager() = default;
MatrixRoomManager::~MatrixRoomManager() = default;

MatrixRoom* MatrixRoomManager::getOrCreateRoom(const std::string& roomId) {
    auto it = m_rooms.find(roomId);
    if (it != m_rooms.end()) return it->second.get();
    auto room = std::make_unique<MatrixRoom>(roomId);
    auto* ptr = room.get();
    m_rooms[roomId] = std::move(room);
    if (m_addedCallback) m_addedCallback(ptr);
    LOGI("Room created: %s (total: %zu)", roomId.c_str(), m_rooms.size());
    return ptr;
}

MatrixRoom* MatrixRoomManager::getRoom(const std::string& roomId) {
    auto it = m_rooms.find(roomId);
    return it != m_rooms.end() ? it->second.get() : nullptr;
}

void MatrixRoomManager::removeRoom(const std::string& roomId) {
    m_rooms.erase(roomId);
    m_favorites.erase(roomId);
    m_directChats.erase(roomId);
    if (m_removedCallback) m_removedCallback(roomId);
}

std::vector<MatrixRoom*> MatrixRoomManager::getAllRooms() {
    std::vector<MatrixRoom*> result;
    for (auto& [id, room] : m_rooms) result.push_back(room.get());
    return result;
}

std::vector<MatrixRoom*> MatrixRoomManager::getRoomsByMembership(RoomMembership membership) {
    std::vector<MatrixRoom*> result;
    for (auto& [id, room] : m_rooms) {
        if (room->info().membership == membership) result.push_back(room.get());
    }
    return result;
}

std::vector<MatrixRoom*> MatrixRoomManager::getDirectChats() {
    std::vector<MatrixRoom*> result;
    for (auto& id : m_directChats) {
        auto* room = getRoom(id);
        if (room) result.push_back(room);
    }
    return result;
}

std::vector<MatrixRoom*> MatrixRoomManager::getSpaces() {
    std::vector<MatrixRoom*> result;
    for (auto& [id, room] : m_rooms) {
        if (room->info().isSpace) result.push_back(room.get());
    }
    return result;
}

std::vector<MatrixRoom*> MatrixRoomManager::getFavoriteRooms() {
    std::vector<MatrixRoom*> result;
    for (auto& id : m_favorites) {
        auto* room = getRoom(id);
        if (room) result.push_back(room);
    }
    return result;
}

void MatrixRoomManager::clear() { m_rooms.clear(); m_favorites.clear(); m_directChats.clear(); }
void MatrixRoomManager::onRoomAdded(std::function<void(MatrixRoom*)> cb) { m_addedCallback = std::move(cb); }
void MatrixRoomManager::onRoomRemoved(std::function<void(const std::string&)> cb) { m_removedCallback = std::move(cb); }
void MatrixRoomManager::setRoomFavorite(const std::string& roomId, bool favorite) {
    if (favorite) m_favorites.insert(roomId);
    else m_favorites.erase(roomId);
}
void MatrixRoomManager::setDirectChat(const std::string& roomId, bool isDm) {
    if (isDm) m_directChats.insert(roomId);
    else m_directChats.erase(roomId);
}

} // namespace matrix_sdk
