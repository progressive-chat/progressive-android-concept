#pragma once
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>
#include <functional>
#include <nlohmann/json.hpp>
#include "matrix_room.hpp"

namespace matrix_sdk {
using json = nlohmann::json;

class MatrixRoomManager {
public:
    MatrixRoomManager(); ~MatrixRoomManager();
    MatrixRoom* getOrCreateRoom(const std::string& roomId);
    MatrixRoom* getRoom(const std::string& roomId);
    void removeRoom(const std::string& roomId);
    std::vector<MatrixRoom*> getAllRooms();
    std::vector<MatrixRoom*> getRoomsByMembership(RoomMembership membership);
    std::vector<MatrixRoom*> getDirectChats();
    std::vector<MatrixRoom*> getSpaces();
    std::vector<MatrixRoom*> getFavoriteRooms();
    int roomCount() const { return static_cast<int>(m_rooms.size()); }
    void clear();
    void onRoomAdded(std::function<void(MatrixRoom*)> cb);
    void onRoomRemoved(std::function<void(const std::string&)> cb);
    void setRoomFavorite(const std::string& roomId, bool favorite);
    void setDirectChat(const std::string& roomId, bool isDm);
private:
    std::map<std::string, std::unique_ptr<MatrixRoom>> m_rooms;
    std::set<std::string> m_favorites;
    std::set<std::string> m_directChats;
    std::function<void(MatrixRoom*)> m_addedCallback;
    std::function<void(const std::string&)> m_removedCallback;
};
} // namespace matrix_sdk
