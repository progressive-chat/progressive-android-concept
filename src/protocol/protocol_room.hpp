#pragma once

#include <QString>
#include <QJsonObject>
#include "protocol_type.hpp"

struct ProtocolRoom {
    QString id;
    QString name;
    QString topic;
    QString avatarUrl;
    int unreadCount = 0;
    int memberCount = 0;
    bool isDirect = false;
    bool isEncrypted = false;
    bool isPublic = false;
    qint64 lastMessageTimestamp = 0;
    QString lastMessagePreview;
    ProtocolType protocolType = ProtocolType::MATRIX;
    QJsonObject metadata;
};
