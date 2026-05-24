#pragma once

#include <QString>
#include <QJsonObject>
#include "protocol_type.hpp"

struct ProtocolMessage {
    qint64 id = 0;
    QString senderId;
    QString senderName;
    QString senderAvatar;
    QString text;
    QString roomId;
    qint64 timestamp = 0;
    ProtocolContentType contentType = ProtocolContentType::TEXT;
    QJsonObject metadata;
    bool isEdited = false;
    bool isEncrypted = false;
    ProtocolType protocolType = ProtocolType::MATRIX;
};
