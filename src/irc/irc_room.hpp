#pragma once

#include <QString>
#include <QStringList>
#include "protocol/protocol_room.hpp"

struct IrcRoom {
    QString channel;
    QString topic;
    int userCount = 0;
    bool isJoined = false;
    QStringList users;

    ProtocolRoom toProtocolRoom() const {
        ProtocolRoom room;
        room.id = channel;
        room.name = channel;
        room.topic = topic;
        room.memberCount = userCount;
        room.isDirect = false;
        room.isEncrypted = false;
        room.isPublic = true;
        room.protocolType = ProtocolType::IRC;
        return room;
    }

    static IrcRoom fromChannel(const QString &channel) {
        IrcRoom room;
        // Ensure channel starts with '#' for consistency
        room.channel = channel.startsWith(QLatin1Char('#'))
            ? channel
            : QLatin1Char('#') + channel;
        return room;
    }
};
