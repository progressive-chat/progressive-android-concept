#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QTime>
#include <QString>
#include <QStringList>

#include "protocol/protocol_type.hpp"

struct ProtocolMessage;

class NotificationManager : public QObject
{
    Q_OBJECT

public:
    static NotificationManager& instance();

    void showNotification(const QString& title, const QString& body,
                          const QString& roomId = QString(),
                          ProtocolType type = ProtocolType::MATRIX);
    void showMessageNotification(const ProtocolMessage& msg);

    bool isQuietHours(const QTime& currentTime) const;
    bool matchesKeywords(const QString& text) const;

    int unreadCount() const;
    int unreadCount(ProtocolType type) const;

    void markRoomRead(const QString& roomId);
    void incrementUnread(const QString& roomId);

    QMap<QString, int> roomUnreadCounts() const;
    QVector<QPair<QString, QString>> notificationHistory() const;

    void loadSettings();
    void saveSettings();

signals:
    void unreadCountChanged(int total);
    void notificationReceived(const QString& title, const QString& body,
                              const QString& roomId);

private:
    NotificationManager(QObject* parent = nullptr);
    ~NotificationManager() override;
    Q_DISABLE_COPY(NotificationManager)

    int computeTotalUnread() const;

    QSystemTrayIcon* m_trayIcon = nullptr;

    QMap<QString, int> m_roomUnreadCounts;
    QMap<QString, ProtocolType> m_roomProtocolMap;

    QVector<QPair<QString, QString>> m_notificationHistory;
    static constexpr int kMaxNotificationHistory = 100;

    bool m_notificationsEnabled = true;
    QTime m_quietHoursStart;
    QTime m_quietHoursEnd;
    QStringList m_keywords;
};
