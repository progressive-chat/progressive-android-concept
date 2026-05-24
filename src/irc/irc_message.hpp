#pragma once

#include <QString>
#include <QStringList>
#include <QDateTime>
#include <QRegularExpression>
#include <QPair>
#include "protocol/protocol_message.hpp"
#include "protocol/protocol_type.hpp"

struct IrcMessage {
    QString prefix;
    QString command;
    QString target;
    QString text;

    QString nick() const {
        if (prefix.isEmpty())
            return {};

        const auto exclamationPos = prefix.indexOf(QLatin1Char('!'));
        if (exclamationPos < 0)
            return prefix;

        return prefix.left(exclamationPos);
    }

    QString user() const {
        if (prefix.isEmpty())
            return {};

        const auto exclamationPos = prefix.indexOf(QLatin1Char('!'));
        if (exclamationPos < 0)
            return {};

        const auto atPos = prefix.indexOf(QLatin1Char('@'), exclamationPos + 1);
        if (atPos < 0)
            return prefix.mid(exclamationPos + 1);

        return prefix.mid(exclamationPos + 1, atPos - exclamationPos - 1);
    }

    QString host() const {
        if (prefix.isEmpty())
            return {};

        const auto atPos = prefix.indexOf(QLatin1Char('@'));
        if (atPos < 0)
            return {};

        return prefix.mid(atPos + 1);
    }

    ProtocolMessage toProtocolMessage(const QString &channel) const {
        ProtocolMessage msg;
        msg.senderId = nick();
        msg.senderName = nick();
        msg.text = text;
        msg.roomId = channel;
        msg.timestamp = QDateTime::currentSecsSinceEpoch();
        msg.contentType = commandToContentType(command);
        msg.protocolType = ProtocolType::IRC;
        msg.isEncrypted = false;
        msg.isEdited = false;
        return msg;
    }

    static IrcMessage fromRawLine(const QString &rawLine) {
        IrcMessage msg;
        if (rawLine.isEmpty())
            return msg;

        QString remaining = rawLine.trimmed();
        int pos = 0;

        // Parse prefix (starts with ':')
        if (remaining.startsWith(QLatin1Char(':'))) {
            const auto spacePos = remaining.indexOf(QLatin1Char(' '));
            if (spacePos < 1)
                return msg;

            msg.prefix = remaining.mid(1, spacePos - 1);
            remaining = remaining.mid(spacePos + 1).trimmed();
        }

        // Parse command
        const auto cmdEnd = remaining.indexOf(QLatin1Char(' '));
        if (cmdEnd < 0) {
            msg.command = remaining.toUpper();
            return msg;
        }

        msg.command = remaining.left(cmdEnd).toUpper();
        remaining = remaining.mid(cmdEnd + 1).trimmed();

        // Parse parameters (target and text)
        // Text part starts with ':' and is everything after it
        const auto colonPos = remaining.indexOf(QLatin1Char(':'));
        if (colonPos >= 0) {
            // Everything before : is target (if present)
            if (colonPos > 0) {
                msg.target = remaining.left(colonPos).trimmed();
            }
            msg.text = remaining.mid(colonPos + 1);
        } else {
            // No trailing text; remaining may contain only target or be empty
            msg.target = remaining;
        }

        // For numeric replies, the target is often the second parameter
        // and text is the rest after that
        if (!msg.target.isEmpty() && msg.text.isEmpty() && msg.command.at(0).isDigit()) {
            const auto targetSpace = msg.target.indexOf(QLatin1Char(' '));
            if (targetSpace >= 0) {
                msg.text = msg.target.mid(targetSpace + 1);
                msg.target = msg.target.left(targetSpace);
            }
        }

        return msg;
    }

    static ProtocolContentType commandToContentType(const QString &ircCommand) {
        if (ircCommand == QLatin1String("PRIVMSG"))
            return ProtocolContentType::TEXT;
        if (ircCommand == QLatin1String("NOTICE"))
            return ProtocolContentType::NOTICE;
        if (ircCommand == QLatin1String("JOIN"))
            return ProtocolContentType::JOIN;
        if (ircCommand == QLatin1String("PART"))
            return ProtocolContentType::PART;
        if (ircCommand == QLatin1String("KICK"))
            return ProtocolContentType::KICK;
        if (ircCommand == QLatin1String("MODE"))
            return ProtocolContentType::SYSTEM;
        if (ircCommand == QLatin1String("TOPIC"))
            return ProtocolContentType::TOPIC;
        if (ircCommand == QLatin1String("QUIT"))
            return ProtocolContentType::PART;
        if (ircCommand == QLatin1String("NICK"))
            return ProtocolContentType::SYSTEM;
        if (ircCommand == QLatin1String("ACTION"))
            return ProtocolContentType::ACTION;

        // CTCP detection in PRIVMSG text is handled by caller
        if (ircCommand == QLatin1String("ERROR"))
            return ProtocolContentType::SYSTEM;
        if (ircCommand == QLatin1String("BAN") || ircCommand == QLatin1String("UNBAN"))
            return ProtocolContentType::BAN;

        return ProtocolContentType::TEXT;
    }
};
