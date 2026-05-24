#pragma once

#include <QString>

enum class ProtocolType { MATRIX, IRC, LEMMY, TELEGRAM, DELTACHAT };

enum class ConnectionState { DISCONNECTED, CONNECTING, CONNECTED, REGISTERED, ERROR };

enum class ProtocolContentType {
    TEXT, IMAGE, VIDEO, AUDIO, FILE, ACTION,
    JOIN, PART, KICK, BAN, TOPIC, NOTICE,
    SYSTEM, LOCATION, STICKER, EMOJI
};

namespace Protocol {
    inline QString protocolTypeToString(ProtocolType type) {
        switch (type) {
        case ProtocolType::MATRIX:    return QStringLiteral("matrix");
        case ProtocolType::IRC:       return QStringLiteral("irc");
        case ProtocolType::LEMMY:     return QStringLiteral("lemmy");
        case ProtocolType::TELEGRAM:  return QStringLiteral("telegram");
        case ProtocolType::DELTACHAT: return QStringLiteral("deltachat");
        }
        return QStringLiteral("unknown");
    }

    inline ProtocolType stringToProtocolType(const QString &str) {
        if (str == QStringLiteral("matrix"))    return ProtocolType::MATRIX;
        if (str == QStringLiteral("irc"))       return ProtocolType::IRC;
        if (str == QStringLiteral("lemmy"))     return ProtocolType::LEMMY;
        if (str == QStringLiteral("telegram"))  return ProtocolType::TELEGRAM;
        if (str == QStringLiteral("deltachat")) return ProtocolType::DELTACHAT;
        return ProtocolType::MATRIX;
    }
}
