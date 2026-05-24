#pragma once

#include <QDialog>
#include <QString>
#include <QVector>

#include "protocol/protocol_message.hpp"
#include "protocol/protocol_room.hpp"
#include "protocol/protocol_type.hpp"

class QLineEdit;
class QListWidget;
class QListWidgetItem;
class QPushButton;
class QLabel;
class QTextEdit;
class QNetworkAccessManager;
class QTimer;

class ForwardMessageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ForwardMessageDialog(const ProtocolMessage &message,
                                  QWidget *parent = nullptr);
    ~ForwardMessageDialog() override = default;

    static QString initialsFromName(const QString &name);
    static QColor avatarColorFromName(const QString &name);
    static QString protocolBadgeText(ProtocolType type);
    static QColor protocolBadgeColor(ProtocolType type);

private slots:
    void onSearchTextChanged(const QString &text);
    void onSearchDebounceTimeout();
    void onForwardClicked();

private:
    struct DestinationItem {
        ProtocolRoom room;
        QString name;
        ProtocolType protocolType;
    };

    void setupUi();
    void populateRooms();
    void filterRooms(const QString &query);
    void addDestinationItem(const DestinationItem &dest);

    static QString formatTimestamp(qint64 timestampMs);

private:
    ProtocolMessage m_message;
    DestinationItem m_originalSource;

    QLabel *m_previewLabel = nullptr;
    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_roomList = nullptr;
    QTextEdit *m_noteEdit = nullptr;
    QPushButton *m_forwardButton = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    QNetworkAccessManager *m_nam = nullptr;
    QTimer *m_searchTimer = nullptr;

    QVector<DestinationItem> m_destinations;
    QVector<QString> m_recentRoomIds;

    static constexpr int kMaxRecentRooms = 20;
    static constexpr int kAvatarSize = 36;
    static constexpr int kPreviewMaxLines = 2;
    static constexpr int kSearchDebounceMs = 300;
};
