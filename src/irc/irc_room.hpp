#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>

namespace progressive_chat {

struct IrcRoomData {
    QString name;
    QString topic;
    QStringList users;
    QStringList ops;
    QStringList voiced;
    QStringList modes;
    QDateTime joinedAt;
    bool hasTopic = false;
};

} // namespace progressive_chat
