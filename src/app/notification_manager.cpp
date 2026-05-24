#include "app/notification_manager.hpp"

#include <QApplication>
#include <QSettings>
#include <QStyle>

#include "protocol/protocol_message.hpp"

NotificationManager& NotificationManager::instance()
{
    static NotificationManager mgr;
    return mgr;
}

NotificationManager::NotificationManager(QObject* parent)
    : QObject(parent)
    , m_trayIcon(new QSystemTrayIcon(this))
{
    m_trayIcon->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxInformation));
    m_trayIcon->show();
    loadSettings();
}

NotificationManager::~NotificationManager() = default;

void NotificationManager::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Notifications"));
    m_notificationsEnabled = settings.value(QStringLiteral("enabled"), true).toBool();
    m_quietHoursStart = settings.value(QStringLiteral("quietHoursStart")).toTime();
    m_quietHoursEnd = settings.value(QStringLiteral("quietHoursEnd")).toTime();
    m_keywords = settings.value(QStringLiteral("keywords")).toStringList();
    settings.endGroup();
}

void NotificationManager::saveSettings()
{
    QSettings settings;
    settings.beginGroup(QStringLiteral("Notifications"));
    settings.setValue(QStringLiteral("enabled"), m_notificationsEnabled);
    settings.setValue(QStringLiteral("quietHoursStart"), m_quietHoursStart);
    settings.setValue(QStringLiteral("quietHoursEnd"), m_quietHoursEnd);
    settings.setValue(QStringLiteral("keywords"), m_keywords);
    settings.endGroup();
    settings.sync();
}

void NotificationManager::showNotification(const QString& title, const QString& body,
                                            const QString& roomId, ProtocolType type)
{
    if (!roomId.isEmpty()) {
        m_roomProtocolMap[roomId] = type;
    }

    m_notificationHistory.prepend(qMakePair(title, body));
    if (m_notificationHistory.size() > kMaxNotificationHistory) {
        m_notificationHistory.resize(kMaxNotificationHistory);
    }

    emit notificationReceived(title, body, roomId);

    if (m_notificationsEnabled && !isQuietHours(QTime::currentTime())) {
        m_trayIcon->showMessage(title, body, QSystemTrayIcon::Information, 5000);
    }
}

void NotificationManager::showMessageNotification(const ProtocolMessage& msg)
{
    const QString title = msg.senderName.isEmpty() ? msg.senderId : msg.senderName;
    const QString body = msg.text;
    showNotification(title, body, msg.roomId, msg.protocolType);
}

bool NotificationManager::isQuietHours(const QTime& currentTime) const
{
    if (!m_quietHoursStart.isValid() || !m_quietHoursEnd.isValid()
        || m_quietHoursStart == m_quietHoursEnd) {
        return false;
    }

    if (m_quietHoursStart < m_quietHoursEnd) {
        return currentTime >= m_quietHoursStart && currentTime <= m_quietHoursEnd;
    }
    return currentTime >= m_quietHoursStart || currentTime <= m_quietHoursEnd;
}

bool NotificationManager::matchesKeywords(const QString& text) const
{
    if (m_keywords.isEmpty()) {
        return false;
    }

    const QString lower = text.toLower();
    for (const QString& keyword : m_keywords) {
        if (lower.contains(keyword.toLower())) {
            return true;
        }
    }
    return false;
}

int NotificationManager::unreadCount() const
{
    return computeTotalUnread();
}

int NotificationManager::unreadCount(ProtocolType type) const
{
    int count = 0;
    for (auto it = m_roomUnreadCounts.constBegin(); it != m_roomUnreadCounts.constEnd(); ++it) {
        const QString& roomId = it.key();
        if (m_roomProtocolMap.value(roomId) == type) {
            count += it.value();
        }
    }
    return count;
}

void NotificationManager::markRoomRead(const QString& roomId)
{
    if (m_roomUnreadCounts.value(roomId, 0) > 0) {
        m_roomUnreadCounts[roomId] = 0;
        emit unreadCountChanged(computeTotalUnread());
    }
}

void NotificationManager::incrementUnread(const QString& roomId)
{
    m_roomUnreadCounts[roomId]++;
    emit unreadCountChanged(computeTotalUnread());
}

QMap<QString, int> NotificationManager::roomUnreadCounts() const
{
    return m_roomUnreadCounts;
}

QVector<QPair<QString, QString>> NotificationManager::notificationHistory() const
{
    return m_notificationHistory;
}

int NotificationManager::computeTotalUnread() const
{
    int total = 0;
    for (int count : m_roomUnreadCounts) {
        total += count;
    }
    return total;
}
