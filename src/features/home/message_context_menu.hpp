#pragma once

#include <QObject>
#include <QString>

#include "protocol/protocol_message.hpp"
#include "protocol/protocol_type.hpp"

class QMenu;
class QWidget;

class MessageContextMenu : public QObject
{
    Q_OBJECT

public:
    static QMenu *create(const ProtocolMessage &msg, QWidget *parent = nullptr);

signals:
    void replyRequested(QString messageId);
    void editRequested(QString messageId);
    void deleteRequested(QString messageId);
    void forwardRequested(QString messageId);
    void reactRequested(QString messageId);
    void reportRequested(QString messageId);
    void pinRequested(QString messageId);
    void unpinRequested(QString messageId);

private:
    explicit MessageContextMenu(QObject *parent = nullptr);
    ~MessageContextMenu() override = default;
    MessageContextMenu(const MessageContextMenu &) = delete;
    MessageContextMenu &operator=(const MessageContextMenu &) = delete;

    void populateMenu(QMenu *menu, const ProtocolMessage &msg);

    static bool messageContainsUrl(const QString &text);
    static bool isOwnMessage(const ProtocolMessage &msg);
    static bool protocolSupportsEdit(ProtocolType type);
    static bool protocolSupportsDelete(ProtocolType type);
    static bool protocolSupportsReact(ProtocolType type);
    static bool protocolSupportsPermalink(ProtocolType type);
    static QString extractUrl(const QString &text);
    static QString messageIdString(const ProtocolMessage &msg);
    static void copyToClipboard(const QString &text);

    ProtocolMessage m_message;
};
