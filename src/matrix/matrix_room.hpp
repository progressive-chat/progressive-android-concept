#pragma once

struct MatrixRoom : ProtocolRoom {
    QString roomId;
    QString name;
    QString topic;
    QString avatarUrl;
    bool isDirect = false;
    bool isEncrypted = false;
    bool isPublic = false;
    int unreadCount = 0;
    int memberCount = 0;
    qint64 lastMessageTimestamp = 0;
    QString lastMessagePreview;
    QString joinRule;
    QString canonicalAlias;

    QJsonObject toJson() const {
        QJsonObject obj;
        obj["room_id"] = roomId;
        obj["name"] = name;
        obj["topic"] = topic;
        obj["avatar_url"] = avatarUrl;
        obj["is_direct"] = isDirect;
        obj["is_encrypted"] = isEncrypted;
        obj["is_public"] = isPublic;
        obj["unread_count"] = unreadCount;
        obj["member_count"] = memberCount;
        obj["last_message_timestamp"] = lastMessageTimestamp;
        obj["last_message_preview"] = lastMessagePreview;
        obj["join_rule"] = joinRule;
        obj["canonical_alias"] = canonicalAlias;
        return obj;
    }

    static MatrixRoom fromJson(const QJsonObject& json) {
        MatrixRoom room;
        room.roomId = json["room_id"].toString();
        room.name = json["name"].toString();
        room.topic = json["topic"].toString();
        room.avatarUrl = json["avatar_url"].toString();
        room.isDirect = json["is_direct"].toBool();
        room.isEncrypted = json["is_encrypted"].toBool();
        room.isPublic = json["is_public"].toBool();
        room.unreadCount = json["unread_count"].toInt();
        room.memberCount = json["member_count"].toInt();
        room.lastMessageTimestamp = static_cast<qint64>(json["last_message_timestamp"].toDouble());
        room.lastMessagePreview = json["last_message_preview"].toString();
        room.joinRule = json["join_rule"].toString();
        room.canonicalAlias = json["canonical_alias"].toString();
        return room;
    }

    static ProtocolRoom toProtocolRoom(const MatrixRoom& room) {
        ProtocolRoom proto;
        proto.roomId = room.roomId;
        proto.name = room.name;
        proto.avatarUrl = room.avatarUrl;
        proto.unreadCount = room.unreadCount;
        proto.lastMessageTimestamp = room.lastMessageTimestamp;
        proto.lastMessagePreview = room.lastMessagePreview;
        return proto;
    }
};
